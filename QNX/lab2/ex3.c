#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

typedef struct {
   int pocz;    // poczatek zakresu
   int kon;      // koniec zakresu
   int numer;  // numer watku
} par_t;

volatile int running_threads = 0;
volatile int p_count = 0;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *count_primes2(void *arg)
{
	par_t *args = (par_t *)arg;

	sleep(1);

	int primes_count = 0;

	printf("Proces: %d, watek: %d\n", getpid(), args->numer);
	int i;
	for(i = args->pocz; i < args->kon; i++) {
		int c;
		int prime = 1;
		for (c = 2; c <= i/2; c++) {
			if (i % c == 0) {
				prime = 0;
				break;
			}
		}
		if (prime && i != 0 && i != 1)
			primes_count++;
	}

	//sleep(15 - args->numer);

	printf("\twatek: %d, poczatek %d, koniec %d, primes %d\n", args->numer, args->pocz, args->kon, primes_count);

	free(arg);

	pthread_mutex_lock(&running_mutex);
	p_count += primes_count;
	running_threads--;
	if (!running_threads) {
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&running_mutex);


	return 0;
}

int main2(int argc, char *argv[])
{
	if(argc != 4) {
		printf("Proper usage: ./lab2 range_start range_end thread_count\n");
		return 0;
	}



	int range_start = atoi(argv[1]);
	int range_end = atoi(argv[2]);
	int threads_count = atoi(argv[3]);
	int range_length = (range_end - range_start) / (threads_count * 4);

	int i = 0;
	pthread_t tid;

	while(1) {
		if(running_threads < threads_count) {
			if (range_start + (i+1)*range_length <= range_end) {
				par_t *args = malloc(sizeof(par_t));
				args->numer = i;
				args->pocz = range_start + i * range_length;
				args->kon = range_start + (i+1) * range_length;
				pthread_create(&tid, NULL, count_primes2, (void *)args);
				pthread_mutex_lock(&running_mutex);
				running_threads++;
				pthread_mutex_unlock(&running_mutex);
				i++;
			} else {
				printf("%d\n", i);
				break;
			}
		}
	}

	pthread_mutex_lock(&running_mutex);
	while(running_threads > 0) {
		pthread_cond_wait( &cond, &running_mutex );
	}
	printf("Liczb pierwszych: %d\n", p_count);
	pthread_mutex_unlock(&running_mutex);




	return 0;
}
