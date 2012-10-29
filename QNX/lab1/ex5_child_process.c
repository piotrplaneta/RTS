#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

	int process_count = atoi(argv[1]);
	int l_count = atoi(argv[2]);

	int i;
	for (i = 0; i < l_count; i++) {
		printf("PID: %d\n", getpid());
		printf("Loop iteration: %d\n", i);
		sleep(1);
	}

	exit(process_count);

}
