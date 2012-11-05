#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>

#define THREADS_COUNT 9
int current_threads_count = 0;
int tid[THREADS_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int absolute_time_to_finish[THREADS_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

int get_thread_index(int t_id){
	int i;
	for(i = 0; i < THREADS_COUNT; i++) {
		if(tid[i] == t_id) {
			break;
		}
	}
	return i;
}

void bubble_sort()
{
  int i, j, temp;

  for (i = (THREADS_COUNT - 1); i > 0; i--)
  {
    for (j = 1; j <= i; j++)
    {
      if (absolute_time_to_finish[j-1] < absolute_time_to_finish[j])
      {
        temp = absolute_time_to_finish[j-1];
        absolute_time_to_finish[j-1] = absolute_time_to_finish[j];
        absolute_time_to_finish[j] = temp;

        temp = tid[j-1];
        tid[j-1] = tid[j];
        tid[j] = temp;
      }
    }
  }
}

void *thread_function(void *arg) {
	int sleep_time = (int) arg;

	sleep(sleep_time);

	tid[get_thread_index(pthread_self())] = -1;
	absolute_time_to_finish[get_thread_index(pthread_self())] = -1;

	return 0;
}

int get_first_empty_index() {
	int i;
	for(i = 0; i < THREADS_COUNT; i++) {
		if(tid[i] == -1) {
			break;
		}
	}
	return i;
}

typedef struct sched_param s_param;

int main(int argc, char *argv[]) {
	char c;
	while(1) {
		scanf("%c", &c);
		if(current_threads_count < THREADS_COUNT) {
			printf("Do U want to create a thread? (1 - YES)\n");
			int confirmation;
			scanf("%d", &confirmation);
			if(confirmation == 1){
				printf("Time in seconds to finish\n");
				int relative_time;
				scanf("%d", &relative_time);
				int index = get_first_empty_index();

				absolute_time_to_finish[index] = time(NULL) + relative_time;
				pthread_create(&tid[index], NULL, thread_function, (void *) relative_time);
				bubble_sort();
				int i = 0;
				while(tid[i] > -1) {
					s_param *args = malloc(sizeof(s_param));
					args->sched_priority = i+1;
					printf("Watek %d, index %d, priorytet %d\n", tid[i], i, i+1);
					pthread_setschedparam(tid[i], SCHED_FIFO, args);
					i++;
				}
			}
		} else {
			printf("Sorry, max threads count attained\n");
		}
	}

	return 0;
}