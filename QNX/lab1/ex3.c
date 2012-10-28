#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>

int main3(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Proper usage: ./lab1 child_processes_count loop_iterations_count\n");
		return EXIT_SUCCESS;
	}

	int p_count = atoi(argv[1]);
	int l_count = atoi(argv[2]);

	int i;
	int process_count = -1;
	for (i = 0; i < p_count; i++) {
		if (fork() == 0) {
			process_count = i;
			break;
		}
	}

	for (i = 0; i < l_count; i++) {
		printf("PID: %d\n", getpid());
		printf("Loop iteration: %d\n", i);
		sleep(1);
	}

	for (i = 0; i < p_count; i++) {
		if(process_count == i) {
			exit(i);
		}
	}


	int pid;
	int status;
	while((pid=wait(&status))!=-1)
		printf("Proces %d zakonczony, status %d\n",pid,WEXITSTATUS(status));
	return 0;
}
