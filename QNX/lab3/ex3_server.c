#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <process.h>
#include <sys/wait.h>

#include "msg_def.h"

#define CHILD "./client"

int main3(int argc, char* argv[]) {
	int chid;
	int pid;
	int rcvid;
	int status;
	kom_t msg;

	int child_count = atoi(argv[1]);

	chid = ChannelCreate(0);
	if(-1 == chid) {
		perror("ChannelCreate()");
		exit(EXIT_FAILURE);
	}

	pid = getpid();
	printf("Server's pid: %d, chid: %d\n", pid, chid);

	int i = 0;
	for(i = 0; i < child_count; i++) {
		int child_pid = fork();
		if (!child_pid) {
			char pid_chars[20];
			char chid_chars[20];
			int base = 10;

			itoa(pid, pid_chars, base);
			itoa(chid, chid_chars, base);
			execl (CHILD, CHILD, pid_chars, chid_chars, (char *)NULL);
		}
	}

	while(1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1) {
			perror("MsgReceive");
			break;
		}
		if(msg.type == 0) {
			printf("\tProces %d wyslal zakonczenie\n", msg.from);
			status = MsgReply(rcvid, EOK, &msg, sizeof(msg) );
			if(-1 == status) {
				perror("MsgReply");
			}
			child_count--;
			if(child_count == 0) {
				break;
			}
			continue;
		}
		printf("%s", msg.text);
		int i;
		int lowcase_count = 0;
		for (i = 0; msg.text[i]; i++) {
			if(msg.text[i] != toupper(msg.text[i]) && msg.text[i] != '\n') {
				msg.text[i] = toupper(msg.text[i]);
				lowcase_count++;
			}
		}
		msg.from = getpid();
		msg.type = 1;

		status = MsgReply(rcvid, EOK, &msg, sizeof(msg) );
		if(-1 == status) {
			perror("MsgReply");
		}
	}

	printf("po petli\n");

	while((pid=wait(&status))!=-1) {
		printf("Proces %d zakonczony, status %d\n",pid,WEXITSTATUS(status));
	}

	return 0;
}

