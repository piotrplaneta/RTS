#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>
#include <string.h>


int main(int argc, char *argv[]) {
	int processes_count = argc - 2;
	char strategy[10];
	strcpy(strategy, argv[argc-1]);
	int s_strategy;

	if (strcmp(strategy, "FIFO") == 0) {
		s_strategy = SCHED_FIFO;
	} else if(strcmp(strategy, "RR") == 0) {
		s_strategy = SCHED_RR;
	} else {
		s_strategy = SCHED_SPORADIC;
	}


	struct sched_param par;
	int str;
	sched_getparam(0, &par);
	sched_setscheduler(0, s_strategy, &par);
	str = sched_getscheduler(0);
	printf("Strategia szeregowania: %d\n", str);
	sleep(5);
	int i;
	for(i = 0; i < processes_count; i++) {
		int pid = fork();

		if (pid == 0) {
			int priority = atoi(argv[i+1]);
			setprio(getpid(), priority);
			int j = 0;
			long long x;
			while (j < 21474836) {
				x = j * j;
				printf("Proces numer: %d\n", i);
			}
			break;
		}
	}

	int pid;
	int status;
	while((pid=wait(&status))!=-1)
		printf("Proces %d zakonczony, status %d\n",pid,WEXITSTATUS(status));
	return (0);
}
