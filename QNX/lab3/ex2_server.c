#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <ctype.h>

#include "msg_def.h"

int main2(void) {
	int chid;
	int pid;
	int rcvid;
	int status;
	mss_t msg;

	chid = ChannelCreate(0);
	if(-1 == chid) {
		perror("ChannelCreate()");
		exit(EXIT_FAILURE);
	}

	pid = getpid();
	printf("Server's pid: %d, chid: %d\n", pid, chid);
	while(1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1) {
			perror("MsgReceive");
			break;
		}
		printf("Proces %d: %s\n", msg.from, msg.text);
		int i;
		int lowcase_count = 0;
		for (i = 0; msg.text[i]; i++) {
			if(msg.text[i] != toupper(msg.text[i]) && msg.text[i] != '\n') {
				msg.text[i] = toupper(msg.text[i]);
				lowcase_count++;
			}
		}
		msg.from = getpid();
		msg.type = LOWCASE_MSG_TYPE;
		msg.count = lowcase_count;

		status = MsgReply(rcvid, EOK, &msg, sizeof(msg) );
		if(-1 == status) {
			perror("MsgReply");
		}
	}

	return 0;
}

