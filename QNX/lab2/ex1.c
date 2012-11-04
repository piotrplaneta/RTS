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

void *count_primes(void *arg)
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


	printf("\twatek: %d, poczatek %d, koniec %d, primes %d\n", args->numer, args->pocz, args->kon, primes_count);
	return (void *) primes_count;
}

int main1(int argc, char *argv[])
{
	if(argc != 4) {
		printf("Proper usage: ./lab2 range_start range_end thread_count\n");
		return 0;
	}



	int range_start = atoi(argv[1]);
	int range_end = atoi(argv[2]);
	int threads_count = atoi(argv[3]);
	int prime_count = 0;
	int range_length = (range_end - range_start) / threads_count;
	pthread_t *tid = (pthread_t *) malloc(threads_count*sizeof(pthread_t));
	par_t *args_tab = (par_t *) malloc(threads_count*sizeof(par_t));

	int i, status;
	for(i = 0; i < threads_count; i++) {
		args_tab[i].numer = i;
		args_tab[i].pocz = range_start + i * range_length;
		args_tab[i].kon = range_start + (i+1) * range_length;
		par_t *args = &args_tab[i];
		pthread_create(&tid[i], NULL, count_primes, (void *)args);
	}
	for(i = 0; i < threads_count; i++) {
		pthread_join(tid[i], (void*)&status);
		printf("watek %d zakonczony, kod powrotu %d\n", tid[i], status);
		prime_count += status;
	}

	printf("Liczb pierwszych: %d\n", prime_count);

	return 0;
}
