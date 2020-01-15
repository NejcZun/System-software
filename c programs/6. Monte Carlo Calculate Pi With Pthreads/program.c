#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define NT 2
#define P 1000000000
#define N 100000

long long Ns = 0;
long long Zs = 0;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void *racunanjePi (void *arg){
	struct timespec myTime;
	clock_gettime(CLOCK_REALTIME,&myTime); // program prevedi s stikalom -lrt
	unsigned int seed = myTime.tv_nsec;
	long long zadetki;
	double x;
	double y;
	while(1){
		zadetki = 0;
		for(int i = 0; i < N; i++){
			x = ((double) rand_r(&seed)) / RAND_MAX; 
			y = ((double) rand_r(&seed)) / RAND_MAX;
			if (x*x + y*y <= 1)zadetki++;	
		}
		pthread_mutex_lock(&mut);
		Zs+=zadetki;
		Ns+=N;
		double pi = 4.0*Zs/Ns;
		printf("%lf\n", pi);
		
		if (Ns >= P){ pthread_mutex_unlock(&mut); break;}
		pthread_mutex_unlock(&mut);
	}
}

int main(){
	pthread_t tid[NT];
	for(int i = 0; i < NT; i++)if (pthread_create(&tid[i], NULL, racunanjePi, NULL) != 0)perror("Kreiranje niti ");
	for(int j = 0; j < NT; j++)pthread_join(tid[j], NULL);
}
