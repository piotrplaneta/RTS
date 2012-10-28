#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>

int main4(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Proper usage: ./lab1 child_processes_count loop_iterations_count\n");
		return EXIT_SUCCESS;
	}

	int p_count = atoi(argv[1]);
	int l_count = atoi(argv[2]);
	int process_count = -1;

	void recursive_fork(int i) {
		if (i < p_count) {
			if(fork() == 0) {
				process_count = i;
				recursive_fork(i+1);
			}
		}
	}

	recursive_fork(0);

	int i;
	for (i = 0; i < l_count; i++) {
		printf("PID: %d\n", getpid());
		printf("Loop iteration: %d\n", i);
		sleep(1);
	}



	if(process_count != p_count-1) {
		int pid;
		int status;
		while((pid=wait(&status))!=-1) {
			printf("Rodzic %d, Proces %d zakonczony, status %d\n",getpid(),pid,WEXITSTATUS(status));
			exit(process_count);
		}

	}
	else {
		exit(process_count);
	}

	return 0;
}
