#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXLINE 64
#define BUFFSIZE 1024


int main(void){
	char *buf[] = {"ls", "-l", NULL};
	char *buf2[] = {"grep", "vaja.c", NULL};
	int fd[2];
	pid_t child1;
	
	if(pipe(fd) < 0)perror("Pipe");
	if( (child1=fork()) <0)perror("Fork");
	if(child1 == 0){
		wait(NULL);
		close(fd[0]);
		// write end of the pipe.
		dup2(fd[1], STDOUT_FILENO);
		execvp(buf[0], buf);
		exit(EXIT_FAILURE);
		
	}
	pid_t child2;
	if( (child2=fork()) < 0)perror("Fork");
	if(child2 == 0){
		close(fd[1]);
		// whenever we read from stdin, actually read from the pipe.
		dup2(fd[0], STDIN_FILENO);
		execvp(buf2[0], buf2);
		exit(EXIT_FAILURE);
	
	}
	close(fd[0]);
	close(fd[1]);
	
	return 0;
}
