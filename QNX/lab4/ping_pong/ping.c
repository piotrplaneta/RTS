#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <sys/dispatch.h>
#include <unistd.h>

#include "msg_def.h"

int main(void) {
	int rcvid;
	int pong_coid = -1;
	int status;
	mss_t msg;
	struct _pulse *mypulse;
	struct _msg_info info;
	name_attach_t *attach;
	int is_connected_to_pong = 0;

	setvbuf(stdout, NULL, _IOLBF, 0); //set IO to stdout to be line buffered

	if ((attach = name_attach(NULL, PING, 0)) == NULL) {
		printf("server:failed to attach name, errno %d\n", errno );
		exit(1);
	}

	while (1) {
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &info);
		if (rcvid == -1) {
			perror("MsgReceive");
			break;
		} else if (rcvid == 0) {
			if (!is_connected_to_pong) {
				if ((pong_coid = name_open(PONG, 0)) == -1) {
					printf("failed to find pong, errno %d\n", errno );
					exit(1);
				}
				is_connected_to_pong = 1;
			}
			mypulse = (struct _pulse *) &msg;
			printf("Pulse code: %d, pulse value %d\n", mypulse->code,
					mypulse->value.sival_int);
			sleep(1);
			status = MsgSendPulse(pong_coid, PULSE_PRIORITY, PULSE_CODE,
					mypulse->value.sival_int + 1);
			if (-1 == status) {
				perror("MsgSendPulse");
				exit(EXIT_FAILURE);
			}
		} else {
			printf("Unexpcted message");

		}
	}

	return 0;
}
