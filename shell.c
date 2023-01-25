// import all import libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readInput(char* userInput,int* flag,int* input_flag,int* output_flag){
	
	// all below staff for ctrl-d
	int c = getc(stdin);
	if(c == EOF ){
		*flag = 1;
		return 0;
	}
	ungetc(c,stdin);

	// set the character pointer so that u can change the userInput Arr
	char* charPtr = userInput;
	*charPtr = c;

	// read till you cant end line
	while(( *charPtr = getchar() ) != '\n' ){
		if(*charPtr == '>'){
			*input_flag = 1;
		}
		if(*charPtr == '<'){
			*output_flag = 1;
		}
		charPtr++;
	}
	
	// end the string
	*charPtr = '\0';

	return 1;
	
}

// split the string and get the arguments
char** splitTheString(char* input_string){
	char* token = strtok( input_string , " ");
	char** arguments = (char**) malloc(sizeof(char*)*100);
	int index = 0;
	while( token != NULL ){
		arguments[index] = (char*) malloc(100*sizeof(char)*(sizeof(token)/sizeof(char)));
		for( int i = 0 ; i < sizeof(token)/sizeof(char) ; i++ ){
				arguments[index][i] = token[i];
		}
		index++;	
		token = strtok(NULL," ");
	}
	arguments[index] = NULL;
	return arguments;
}

void executeCommand(char* path,char** argv){
	int pid = fork();
	if(pid == 0) {
			execv(path, argv);
	}else{
		wait(0);
	}
}



int main(){
	
	// input string
	char input_string[1000];

	// whenever user wants to change the prompt string
	char prompt_string[256];

	// string containing current working directory
	char cwd[256];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
    	perror("getcwd() error");
    
	// set up the path
	char path[5000];
    char str[5000] = "/usr/bin";
    //char buf[5000];

	// default prompt string
	prompt_string[0] = '>';
	prompt_string[1] = '>';
	prompt_string[2] = '\0';

	// int i
	int i,j;
	int while_end_flag = 0;

	// take the input in while loop
	while(1){

		int input_redirection_flag = 0;
		int output_redirection_flag = 0;

		// show the start point
		printf("%s %s ",cwd,prompt_string);

		// read the string input from user
		if(readInput(input_string,&while_end_flag,&input_redirection_flag,&output_redirection_flag)){

			
	
			// if the input from user is exit then exit
			if(strncmp(input_string,"exit",4) == 0 ){
				return 0;
			}

			// if user asks to change the prompt
			if(strncmp(input_string,"PS1",3) == 0){
				
				if(input_string[6] == 'w' && input_string[7] == '$'){
					//printf("\n%c\n",input_string[7]);
					if (getcwd(cwd, sizeof(cwd)) == NULL){
						perror("getcwd() error");
					}
					prompt_string[0] = '>';
					prompt_string[1] = '>';
					prompt_string[2] = '\0';
				}else{
					i = 0;
					while(input_string[i]){
						if( i > 4 && input_string[i] != 34 ){
							prompt_string[i-5] = input_string[i];
						}
						i++;
					}
					cwd[0]='\0';
				}
			}

			// if user want to change directory
			if( strncmp(input_string,"cd",2) == 0 ){
				
				char target_directory[5000];
				if(strlen(input_string)-2){
					for( i = 3 ; i < strlen(input_string) ; i++){
						target_directory[i-3] = input_string[i];
					}
					target_directory[i-3]='\0';
					chdir(target_directory);
				}else{
					char* target_direct = getenv("HOME");
					printf("%s\n",target_direct);
					chdir(target_direct);
				}
				if (getcwd(cwd, sizeof(cwd)) == NULL)
    				perror("getcwd() error");
				
			}else{
				// split the string and below contain command and its arguments
				char** arguments = splitTheString(input_string);
				for( i = 0 ; i < strlen(str) ; i++){
					path[i] = str[i];
				}
				path[i++] = '/';

				for ( j = 0 ; j < strlen(arguments[0]) ; j++){
					path[i++] = arguments[0][j];
				}
				path[i++]='\0';

				// when > in command
				if(input_redirection_flag == 1){
					//printf("%s %s %s\n",arguments[0],arguments[1],arguments[2]);
					int null_index = 0;
					for( i = 0 ; arguments[i] ; i++){
						if(arguments[i][0] == '>'){
							null_index = i;
							break;
						}
					}
					arguments[null_index] = '\0';
					int pid = fork();
					if( pid == -1){
						perror("Error\n");
					}
					if( pid == 0){
						close(1);
						int fd = open(arguments[null_index+1],O_CREAT | O_WRONLY,1000);
						//printf("Here1\n");
						if(execv(path,arguments) == -1){
							printf("Error exec\n");
						};
						//printf("Here2\n");
					}else{
						wait(0);
					}
				}	
				else if(strncmp(input_string,"PATH=",5) == 0){
					printf("USER WANTS TO CHANGE THE PATH\n");
					for( i = 5 ; input_string[i] ; i++){
						str[i-5] = (input_string[i]);
					}
					str[i-5] = '\0';
				}

				// when < is in command
				else if(output_redirection_flag == 1){

					int null_index = 0;
					for( i = 0 ; arguments[i] ; i++){
						if(arguments[i][0] == '>'){
							null_index = i;
							break;
						}
					}
					arguments[null_index+1] = '\0';
					int pid = fork();
					if( pid == -1){
						perror("Error\n");
					}
					if( pid == 0){
						close(0);
						int fd = open(arguments[null_index+2],O_RDONLY,1000);
						if(execv(path,arguments) == -1){
							printf("Error exec\n");
						};
					}else{
						wait(0);
					}
				}else{
					printf("%s\n",path);
					executeCommand(path,arguments);
				}
			}
		}

		if(while_end_flag){
			printf("BYE\n");
			return 0;	
		}
	}
	return 0;
}
