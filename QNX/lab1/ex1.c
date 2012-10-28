#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#define PATH_MAX 1024

int main1(int argc, char *argv[]) {
	printf("PID: %d\n", getpid());
	printf("PPID: %d\n", getppid());
	printf("PGID: %d\n", getpgrp());
	printf("SID: %d\n", getsid(getpid()));
	printf("UID: %d\n", getuid());
	printf("GID: %d\n", getgid());
	printf("ENV (PATH): %s\n", getenv("PATH"));

	char* cwd;
	char buff[PATH_MAX + 1];
	cwd = getcwd( buff, PATH_MAX + 1 );
	if( cwd != NULL ) {
		printf( "DIRECTORY: %s\n", cwd );
	}

	printf("PRIORITY: %d\n", getprio(getpid()));
	printf("SCHEDULER: %d\n", sched_getscheduler(getpid()));
	return 0;
}
