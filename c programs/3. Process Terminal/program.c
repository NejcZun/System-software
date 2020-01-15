#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>


#define MAXLINE 1024
#define BUFFSIZE 1024

void parse(char *niz, char **p){
    int i=0, j=0;
    while( niz[i] != '\0'){
        if ( niz[i] != ' ' && niz[i] != '\t' && niz[i] != '\0' && niz[i] != '\n' ){ //poln znak
            if ( i > 0) {
                if ( ! ( niz[i-1] != ' ' && niz[i-1] != '\t' && niz[i-1] != '\0' ) ) // prejšnji prazen
                    p[j++] = &niz[i];
            }else p[j++] = &niz[i];
        }else niz[i] = '\0';
        i++;
    }
    p[j] = (char *)0;
}

void main(void){
	char buf[BUFFSIZE];
	char *args[MAXLINE];
	while (fgets(buf, MAXLINE, stdin) != NULL) {
	    parse(buf, args);
	    pid_t pid;
	    if (pid = fork() < 0) {perror("Error");} 
	    else { if (pid == 0){ execvp(args[0], args);}}
    }

}