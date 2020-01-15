#define _DEFAULT_SOURCE
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

#define N 5
#define MEMSIZE 512

int flag = 1;
char prev_str[MEMSIZE] = "";

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
} sem_args;

struct my_message {
	long mtype;
	char mtext[MEMSIZE];
};



void beri_sporocilo(void *buffer, int sv){
	char *str[MEMSIZE];
	struct my_message message;
	message.mtype = 1;
	memcpy(str, buffer, MEMSIZE); //message iz bufferja
	
	if(strlen((char*) str)==0){
		flag = 0; //k pride \0
		printf("Posredujem: EOF\n");
		strcpy(message.mtext, "\0");
		if(msgsnd(sv, &message, 1, 0) < 0)perror("Napaka pri posiljanju v vrsto");
	}else{
		if(strcmp((char*)str, prev_str) != 0){ //da je string razlicn
			printf("Posredujem: %s\n", (char*) str);
			strcpy(message.mtext, (char*) str);
			int len = strlen(message.mtext);
			if(msgsnd(sv, &message, len-1, 0) < 0)perror("Napaka pri posiljanju v vrsto");
			bzero(prev_str, MEMSIZE); //zbrisemo prej_string
			strcpy(prev_str, (char*) str); //kopiramo novga v prejsnga
		}
	}
	 
}

int main(int argc, char *argv[]){
	int shm, sem, semcontrol, sv;
	key_t key;
	void *pointer;
	int exit;
	sem_args.val = 1;
	
	struct sembuf sestevanje;
	struct sembuf odstevanje;
	
	/* Pristevanje */
	sestevanje.sem_num = 0;
	sestevanje.sem_op = 1;
	sestevanje.sem_flg = 0;
	
	/* Odstevanje */
	odstevanje.sem_num = 0;
	odstevanje.sem_op = -1;
	odstevanje.sem_flg = 0;
	
	/* priklop na skupnega*/
	if((key = ftok("/tmp", 'a')) < 0) perror("Napaka pri generiranju kljuca");
	if((shm = shmget(key, MEMSIZE, 0664)) < 0) perror("Napaka pri deljenem polnilniku");
	if((sem = semget(key, 1, 0664)) < 0) perror("Napaka pri semaforju");
	
	/*ustvari sporocilno vrsto*/
	if((sv = msgget(key, 0664 | IPC_CREAT)) < 0) perror("Napaka pri ustvarjanju sporocilne vrste");
	if((pointer = shmat(shm,NULL, SHM_RDONLY)) < 0) perror("Napaka pri povezovanju na deljen pomnilnik");
	
	while(flag){
		if( semop(sem, &odstevanje, 1) < 0) perror("Napaka pri odstevanje vrednosti semaforja");
		beri_sporocilo(pointer, sv);
		if( semop(sem, &sestevanje, 1) < 0) perror("Napaka pri sestevanje vrednosti semaforja");
		sleep(1);
	}
	if((exit = shmdt(pointer)) < 0) perror("Napaka pri zapiranju deljenega pomnilnika");
	if((exit = semctl(sem, 1, IPC_RMID, sem_args)) < 0) perror("Napaka pri zapiranju semaforja");
	
	
	return 0;
}