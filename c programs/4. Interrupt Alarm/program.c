#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


#define MAXLINE 64
#define BUFFSIZE 1024

// man strtok

int counter = 0;

void sig_int(int signo){
	if(counter == 0){		// first press inform and increment by one
		printf("\nZa izhod iz programa ponovno pritisnite CTRL+C v roku 3 sekund.\n ");
		alarm(3);
		counter += 1;
	}
	else{
		printf("\nNasvidenje! \n");	// we murder the cli.c
		exit(13);
	}
}

void sig_alarm(int signo){		// reset the value of presses
	counter = 0;
}

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

int main(void){
	char buf[BUFFSIZE];
	char *args[MAXLINE];
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		//ujame interrupt
		if (signal(SIGINT, sig_int) == SIG_ERR)perror("Signal error");

		if (signal(SIGALRM, sig_alarm) == SIG_ERR)	// catch the alarm for reset
			perror("Alarm error");

		parse(buf, args);
		pid_t pid;
		int status;
		pid = fork();
		if(pid == 0){
			int jeba = execvp(args[0], args); //zazene argumente
			if(jeba < 0) perror("Napakica");
			return 0;
		} else{
			wait(&status);
		}
	}
	return 0;
}
