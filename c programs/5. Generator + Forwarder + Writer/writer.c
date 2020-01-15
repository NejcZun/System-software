#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <err.h>
#include <unistd.h>
#include <string.h>

#define MEMSIZE 512
int flag = 1;
struct my_message {
	long mtype;
	char mtext[MEMSIZE];
};

void zapisi_sporocilo(FILE *fp, int sv){
	struct my_message message;
	long msgtype = 0;
	if(msgrcv(sv, &message, sizeof(message.mtext), msgtype, 0) < 0)perror("Napaka pri branju iz sporocilne vrste");
	if(fputs(message.mtext, fp) == EOF) perror("Napaka pri zapisovanju");
	if(strcmp(message.mtext, "\0") == 0){
		flag=0;
		printf("Zapisujem: EOF\n");
	}else printf("Zapisujem: %s\n", message.mtext);
}

int main(int argc, char *argv[]){
	int sem, sv;
	key_t key;
	FILE *fp;
	int exit;
	if(argc != 2){ perror("Sintax error: ./zapisovalec.out <ime datoteke>"); return 0;} 
	
	/*ustvari datoteke za pisanje append */
	if ((fp = fopen(argv[1], "a+")) < 0) perror("Napaka pri ustvarjanju datoteke");
	
	
	if((key = ftok("/tmp", 'a')) < 0) perror("Napaka pri generiranju kljuca");
	if((sv = msgget(key, IPC_EXCL | 0664)) < 0) perror("Napaka pri odpiranju sporocilne vrste");
	while(flag){
		zapisi_sporocilo(fp, sv);
	}
	if (msgctl(sv, IPC_RMID, NULL) == -1)perror("Napaka pri brisanju sporocilne vrste");
	fclose(fp);
	return 0;
}