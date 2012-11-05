#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t reading_room_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t num_readers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readers = PTHREAD_COND_INITIALIZER;
pthread_cond_t writers = PTHREAD_COND_INITIALIZER;
int num_readers = 0;
int reading_room_taken = 0;

void *reader(void *args) {
	int i = (int) args;
	sleep(i % 500);
	pthread_mutex_lock(&reading_room_mutex);
	if (!reading_room_taken) {
		reading_room_taken = 1;
		pthread_cond_broadcast(&readers);
	}
	else {
		pthread_cond_wait(&readers, &reading_room_mutex);
	}

	pthread_mutex_lock(&num_readers_mutex);
	num_readers++;
	if(num_readers == 1) {
		reading_room_taken = 1;
		pthread_cond_broadcast(&readers);
	}
	pthread_mutex_unlock(&num_readers_mutex);

	pthread_mutex_unlock(&reading_room_mutex);

	printf("start reading %d\n", i);
	sleep(1);
	printf("finish reading %d\n", i);

	pthread_mutex_lock(&num_readers_mutex);
	num_readers--;
	if (num_readers == 0) {
		pthread_mutex_lock(&reading_room_mutex);
		reading_room_taken = 0;
		pthread_mutex_unlock(&reading_room_mutex);
		pthread_cond_signal(&writers);
	}
	pthread_mutex_unlock(&num_readers_mutex);
	return (void *)0;
}

void *writer(void *args) {
	int i = (int) args;
	sleep(i % 10);
	pthread_mutex_lock(&reading_room_mutex);
	if (!reading_room_taken) {
		reading_room_taken = 1;
	} else {
		pthread_cond_wait(&writers, &reading_room_mutex);
	}
	reading_room_taken = 1;
	pthread_mutex_unlock(&reading_room_mutex);

	printf("start writing %d \n", i);
	sleep(1);
	printf("finish writing %d \n", i);

	pthread_mutex_lock(&reading_room_mutex);
	reading_room_taken = 0;
	pthread_mutex_unlock(&reading_room_mutex);
	pthread_cond_signal(&readers);
	return (void *)0;
}

int main() {
	pthread_t threads[10100];

	int i;

	for(i = 0; i <= 100; i++) {
		pthread_create(&threads[i], NULL, writer, (void *)i);
	}

	for(i = 0; i <= 10000; i++) {
		pthread_create(&threads[i+100], NULL, reader, (void *)i);
	}

	for(i = 0; i <= 10100; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
