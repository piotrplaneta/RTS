#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>

#define CHILD "./ex5ChildProcess"

int main5(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Proper usage: ./lab1 child_processes_count loop_iterations_count\n");
		return EXIT_SUCCESS;
	}

	int p_count = atoi(argv[1]);
	int l_count = atoi(argv[2]);

	int status;
	pid_t pid;
	int i;
	for (i = 0; i < p_count; i++) {
		pid = fork();
		if (pid == 0)
		{
			/* This is the child process.  Execute the child command. */
			char process_count[20];
			char loop_count[20];
			int base = 10;

			itoa(i, process_count, base);
			itoa(l_count, loop_count, base);


			execl (CHILD, CHILD,  process_count, loop_count, (char *)NULL);
			_exit (EXIT_FAILURE);
		}

	}

	while((pid=wait(&status))!=-1) {
		printf("Rodzic %d, Proces %d zakonczony, status %d\n",getpid(),pid,WEXITSTATUS(status));
	}
	return 0;

}
