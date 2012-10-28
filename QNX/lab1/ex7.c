#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>

#define CHILD "./ex5ChildProcess"

int main7(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Proper usage: ./lab1 loop_iterations_count\n");
		return EXIT_SUCCESS;
	}

	int l_count = atoi(argv[1]);

	int status;
	pid_t pid;

	int res;

	char process_count[20];
	char loop_count[20];
	int base = 10;

	itoa(l_count, loop_count, base);
	itoa(0, process_count, base);


	res = spawnl(P_WAIT, CHILD, CHILD, process_count, loop_count, (char *)NULL);
	if (res<0)
	{
		perror("Blad funkcji spawn");
		exit(EXIT_FAILURE);
	}

	printf("I'm a parent after P_WAIT\n");

	itoa(1, process_count, base);
	res = spawnl(P_NOWAIT, CHILD, CHILD, process_count, loop_count, (char *)NULL);
	if (res<0)
	{
		perror("Blad funkcji spawn");
		exit(EXIT_FAILURE);
	}

	printf("I'm a parent after P_NOWAIT\n");

	itoa(2, process_count, base);
	res = spawnl(P_NOWAITO, CHILD, CHILD, process_count, loop_count, (char *)NULL);
	if (res<0)
	{
		perror("Blad funkcji spawn");
		exit(EXIT_FAILURE);
	}

	printf("I'm a parent after P_NOWAITO\n");

	while((pid=wait(&status))!=-1) {
		printf("Rodzic %d, Proces %d zakonczony, status %d\n",getpid(),pid,WEXITSTATUS(status));
	}
	return 0;

}
