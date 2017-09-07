//Author: Siqing Xu
//Honor Pledge: Siqing Xu
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

//allow 200 char for one command
#define SIZE 200
#define NEWLINE '\n'
#define SPACE ' '
#define TERMINATE '\0'

char prompt[3] = "% ";
char input[SIZE];

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

	/*execute the command with arguments*/
	int res;
	if((res = execvp(argv[0], argv)) < 0){
		/*exec command should not return, an error occurs
		print error message and exit the child process*/
		printf("Failed: invalid command\n");
		exit(1);
	}

}

int main(){
	char ch;
	int i = 0;
	/*print out the first prompt*/
	printf("%s", prompt);

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
			int pid;
			if((pid = fork()) < 0){
				/*unsuccessful fork, go to next prompt*/
				printf("Failed: unsuccessful fork\n");
				printf("%s", prompt);
				i = 0;
				continue;
			}
			if(pid == 0){
				/*child process*/
				parseCommand(input, &i);
			}else{
				/*parent process
				wait for child process to terminate and start new prompt*/
				wait(NULL);
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