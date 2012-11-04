#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define SIZE 5

pthread_mutex_t region_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t space_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t data_available = PTHREAD_COND_INITIALIZER;

int products[SIZE];
int size = 0;
int front, rear = 0;

void add_product(int i) {
	products[rear] = i;
	rear = (rear + 1) % SIZE;
	size++;
}

int get_product() {
	int v;
	v = products[front];
	front = (front + 1) % SIZE;
	size--;
	return v;
}

void *producer() {
	int i = 0;
	while (1) {

		pthread_mutex_lock(&region_mutex);
		if (size == SIZE) {
			pthread_cond_wait(&space_available, &region_mutex);
		}
        //sleep(1);
		add_product(i);
		printf("produced %d\n", i);
		pthread_cond_signal(&data_available);
		pthread_mutex_unlock(&region_mutex);
		i = i + 1;
	}
	return 0;
}

void *consumer() {
	int i, v;
	for (i = 0; i < 100; i++) {
		pthread_mutex_lock(&region_mutex);
		if (size == 0) {
			pthread_cond_wait(&data_available, &region_mutex);
		}
		sleep(1);
		v = get_product();
		pthread_cond_signal(&space_available);
		pthread_mutex_unlock(&region_mutex);
		printf("got %d\n", v);
	}
	return 0;
}

int main3() {
	pthread_t producer_thread;
	pthread_t consumer_thread;

	pthread_create(&consumer_thread, NULL, consumer, NULL);
	pthread_create(&producer_thread, NULL, producer, NULL);
	pthread_join(consumer_thread, NULL);
	return 0;
}


