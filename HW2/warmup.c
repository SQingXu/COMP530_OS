//Author: Siqing Xu
//Honor Pledge: Siqing Xu
#include <stdio.h>
#include <stdlib.h>

#define SIZE 81
#define NEWLINE  '\n'
#define ASTERISKS '*'
#define UPCHAR '^'
#define SPACE ' '

void clearAndPrint(char arr[],int* i){
	arr[SIZE - 1] = NEWLINE;
	printf("%.*s",SIZE, arr);
	*i = 0;
    exit(0);
	return;
}

int main(){
	char ch;
	char pre_ch = '\0'; 
	char string[SIZE];
	int i = 0;
    while((ch = getchar()) != EOF){
    	if(pre_ch == ASTERISKS && ch == ASTERISKS){
    		string[i] = UPCHAR;
    		i++;
    		ch = UPCHAR;
    	}else if(ch == ASTERISKS){
    	}else if(pre_ch == ASTERISKS){
    		string[i] = ASTERISKS;
    		i++;
    		if(i == SIZE-1){
    			clearAndPrint(string, &i);
    		}
    		if(ch == NEWLINE){
    			string[i] = SPACE;
    		}else{
    			string[i] = ch;
    		}
    		i++;
    	}else{
    		if(ch == NEWLINE){
    			string[i] = SPACE;
    		}else{
    			string[i] = ch;
    		}
    		i++;
    	}
    	pre_ch = ch;
    	if(i == SIZE-1){
    		clearAndPrint(string, &i);
    	}

    }
    if(pre_ch == ASTERISKS){
        string[i] = ASTERISKS;
        i++;
        if(i == SIZE - 1){
                clearAndPrint(string, &i);
        }
    }
    return 0;
} 
