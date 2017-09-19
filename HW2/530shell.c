//Author: Siqing Xu
//Honor Pledge: Siqing Xu
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


//allow 200 char for one command
#define SIZE 200
#define PATH_MAX 50
#define NEWLINE '\n'
#define SPACE ' '
#define COLUMN ':'
#define TERMINATE '\0'

char prompt[3] = "% ";
char input[SIZE];
int pid;

static void sigHandler(int sig);
void parsePath(char* pathes[], char* comb, int* path_num);
void parseCommand(char input[], int* i){
	/*set variables for the parsing*/
	int comm_len = *i;
	int input_i;

	/*variables to argument list
	the number of arguements can be no more than the length of input*/
	char* argv[comm_len+1];
	int arg_index = 0;

    /*variables to a single argument*/
	char arg[comm_len];
	int arg_len = 0;

	for(input_i = 0; input_i < comm_len; input_i++){
		/*within the arguments region*/
		if(input[input_i] == SPACE){
			/*read of one argument ends*/
			arg[arg_len] = TERMINATE;
			argv[arg_index] = (char *) malloc((arg_len + 1) * sizeof(char));
			strcpy(argv[arg_index], arg);
			arg_index++;
			arg_len = 0;
			continue;
		}
		arg[arg_len] = input[input_i];
		arg_len++;
	}
	/*add the last argument to the argument array and add NULL*/
	arg[arg_len] = TERMINATE;
	argv[arg_index] = (char *) malloc((arg_len + 1) * sizeof(char));	
	strcpy(argv[arg_index], arg);
	argv[arg_index + 1] = NULL;

    /*before parse the pathes
    if command itself is a valid filename then execute it directly*/
	int res;
	struct stat stbuf;
	if(stat(argv[0], &stbuf) == 0){
		/*if valid filename*/
		/*execute the command with arguments*/
		if((res = execv(argv[0], argv)) < 0){
		     /*exec command should not return, an error occurs
		     print error message and exit the child process*/
			printf("Failed: error when executing\n");
			exit(1);
		}
	}

	/*parse all pathes from getenv function*/
	char* pathes[PATH_MAX];
	int path_num;
	parsePath(pathes, getenv("PATH"),&path_num);
	
	/*use stat function to loop through all the possible path
	once find the valid one return the valid */
	char* output_path = NULL;
	int p;
	int valid = 0;
	for(p = 0; p < path_num; p++){
		struct stat stbuf;
		int len = strlen(pathes[p]) + strlen(argv[0]) + 1;
		char path[len];
		strcpy(path, pathes[p]);
		strcat(path, "/");
		strcat(path, argv[0]);
                /*for debugging*/
		//printf("%s\n", path);
		if(stat(path, &stbuf) == 0){
			/*valid path*/
			valid = 1;
                        if(output_path == NULL){
                          output_path = (char*) malloc(len * sizeof(char));
                        }
			strcpy(output_path, path);
                        break;
		}
	}
	/*if no path is valid after iteration, exit and output error msg*/
	if(valid == 0){
		printf("Failed: file not found\n");
		exit(1);
	}
	
	/*execute the command with arguments*/
	if((res = execv(output_path, argv)) < 0){
		/*exec command should not return, an error occurs
		print error message and exit the child process*/
		printf("Failed: error when executing\n");
		exit(1);
	}

}

int main(){
	char ch;
	int i = 0;
	/*print out the first prompt*/
	printf("%s", prompt);

	signal(SIGINT, sigHandler);
	signal(SIGTSTP, sigHandler);

	while((ch = getchar()) != EOF){
		if(i == SIZE){
			/*command length reach the size limit*/
			printf("Failed: limit of command length reached\n");
			/*read the part that excceed the length until the new line*/
			while((ch = getchar()) != NEWLINE){
			}
			printf("%s", prompt);
			i = 0;
			continue;
		}
		if(ch == NEWLINE){
			/*reach the end of one line of command
			start to fork*/
			//int pid;
			
			/*before the child process terminates
			control-z: define a handler which does nothing for parent process, sigaction
			control-c: ignore the signal*/
			
			if((pid = fork()) < 0){
				/*unsuccessful fork, go to next prompt*/
				printf("Failed: unsuccessful fork\n");
				printf("%s", prompt);
				i = 0;
				continue;
			}


			if(pid == 0){
				/*child process*/
				/*Don't ignore control-c and control-z in child process*/
				parseCommand(input, &i);
			}else{
				/*parent process
				wait for child process to terminate and start new prompt*/
				wait(NULL);
				/*after child process exit, set both signals to default*/
				i = 0;
				printf("%s", prompt);
				continue;
			}
		}

		input[i] = ch;
		i++;

	}
	return 0;
}

void parsePath(char* pathes[], char* comb, int* path_num){
	/*variables for one whole path string*/
	int total_len = strlen(comb);
	int c_index;
	int p_index = 0;

	/*set variables to store each single path
	strlen does not include the null terminator*/
	char path[total_len+1];
	int path_len = 0;

	for(c_index = 0; c_index < total_len; c_index++){
		if(comb[c_index] == COLUMN){
			/*the end of one path
			add null terminator and copy it into the array*/
			path[path_len] = TERMINATE;
			pathes[p_index] = (char *) malloc((path_len + 1) * sizeof(char));
			strcpy(pathes[p_index], path);
			path_len = 0;
			p_index++;
			if(p_index == PATH_MAX){
				/*path limit reached, no more pathes can be parsed*/
				*path_num = PATH_MAX;
				return;
			}
			continue;
		}
		path[path_len] = comb[c_index];
		path_len++;
	}
	/*deal with the last path without column ending*/
	path[path_len] = TERMINATE;
	pathes[p_index] = (char *) malloc((path_len + 1) * sizeof(char));
	strcpy(pathes[p_index], path);
	*path_num = p_index + 1;
	return;
}

static void sigHandler(int sig){
	switch(sig){
		/*Ctrl-C*/
		case SIGINT:
		if(pid != 0){
			kill(pid, SIGINT);
		}
		break;
		/*Ctrl-Z*/
		case SIGTSTP:
		// printf("Process %d receive SIGTSTP signal\n",getpid());
		// fflush(stdout);
		if(pid != 0){
			kill(pid, SIGINT);
		}
		break;
		default: break;
	}
	return;
}

// void sigZPHandler(int sig){
// 	if(sig == SIGTSTP){
// 	}
// }
