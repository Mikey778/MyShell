#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUFSIZE 1024

struct cmd
  {
    int redirect_in;     
    int redirect_out;    
    int redirect_append;
    int background;      
    int piping;         
    char *infile;        
    char *outfile;       
    char *argv1[10];     
    char *argv2[10];     
  };

int cmdscan(char *cmdbuf, struct cmd *com);
int status, file_APPEND, file_OUTPUT, file_INPUT;
void redirectCheck();
struct cmd command;

int main() {
    
char buf[BUFSIZE];
pid_t pid, pid1, pid2;
int fd[2];

// get USER name and print
printf("--------- Welcome %s ---------\n",getenv("USER"));	
printf("[%s]: ", getenv("USER"));

// while stdin != Null run while loop
while((fgets(buf,BUFSIZE,stdin) != NULL)) {


	//if error from cmdscan.c print error and continue
	if(cmdscan(buf,&command) == -1) {
        	printf("ILLEGAL FORMAT\n");
		printf("[%s]: ", getenv("USER"));
		continue;
	}//end if error from cmdscan.c
	
	//if user types exit terminate shell
	if(strcmp(command.argv1[0],"exit") == 0) {
		exit(0);
        }//end if exit 
	
	// create first child process
    	if((pid = fork()) < 0){
       		perror("FORK ERROR\n");
	}//end if fork error check
	// if child process
    	else if (pid == 0) {
		
		//if backgrounding = true create another child process pid2
		if (command.background){
			pid2 = fork(); 
			if(pid2 < 0){
                		perror("FORK ERROR\n");
			}//end if pid2 error
		}//end if backgrounding
		
		//if backgrounding and parent of child pid2 exit 
		//pid2 is now adopted by init
		if(command.background && (pid2 != 0)){
			exit(0);
		}//end if backgrounding and pid2 !=0
		
		// if not backgrounding or backgrounding and child pid2
		if(!command.background || (command.background && pid2 == 0)){
		
			//if piping
			if(command.piping){
				pipe(fd);
				pid1 = fork();	
        			
				//if pid1 is the parent process
				if (pid1 > 0) {

					redirectCheck();
					
					close(fd[1]);
					dup2(fd[0], STDIN_FILENO);
      					execvp(command.argv2[0],command.argv2);

				}//end if pid1 is parent process
				
				// if pid1 is the child process
				else if( pid1 == 0){

					redirectCheck();
       		
					close(fd[0]);
       					dup2(fd[1],STDOUT_FILENO);
					execvp(command.argv1[0],command.argv1);
				}//end else if child process
			}//end if piping
			
			//if !piping
			if(!command.piping){
	
				redirectCheck();

				// error check
                		if(execvp(command.argv1[0],command.argv1) == -1){
					printf("NOT A VALID USAGE\n");
					exit(1);
				}//end if error check
		
			//execute arguments
			execvp(command.argv1[0],command.argv1);
			}//end if !piping
        	exit(0);
		}//end if not backgrounding or backgrounding and child pid2
    }//end else if child 
	else {
        if(wait(&status) != pid)
            	perror("WAIT ERROR");
		printf("[%s]: ", getenv("USER")); 
    }//end else
}//end while loop

    return 0;

}//end main

//redirection check and execution
void redirectCheck(){

		//append redirection
		if(command.redirect_append) {
                	file_APPEND = open(command.outfile, O_CREAT | O_RDWR | O_APPEND, 0770);
                	if(file_APPEND < 0){
                		perror("OPEN ERROR: APPEND");
                	}//end error check
		dup2(file_APPEND, STDOUT_FILENO);
                close (file_APPEND);
                }//end append redirection
		
		//output redirection
		else if(command.redirect_out && !command.redirect_append) {
                	file_OUTPUT = open (command.outfile, O_CREAT | O_RDWR | O_TRUNC, 0770);
                	if(file_OUTPUT < 0){
				perror("OPEN ERROR: OUTPUT");
			}//end error check
		dup2(file_OUTPUT, STDOUT_FILENO);
                close(file_OUTPUT);

                }//end output redirection 

		//input redirection
		if (command.redirect_in){
			file_INPUT = open(command.infile, O_RDONLY);
			if(file_INPUT < 0){
                		perror("OPEN ERROR: INPUT");
                	}//end error check
		dup2(file_INPUT,STDIN_FILENO);
		close(file_INPUT);
		
		}//end input redirection
}//end redirectCheck()

