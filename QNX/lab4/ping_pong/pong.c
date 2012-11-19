//kod klienta
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <unistd.h>

#include "msg_def.h"

int main(int argc, char* argv[]) {
	int ping_coid;
	int status; //status return value used for ConnectAttach and MsgSend
	struct _pulse *mypulse;
	struct _msg_info info;
	mss_t msg;
	name_attach_t *attach;

	setvbuf(stdout, NULL, _IOLBF, 0);

	if ((attach = name_attach(NULL, PONG, 0)) == NULL) {
		printf("server:failed to attach name, errno %d\n", errno );
		exit(1);
	}

	if ((ping_coid = name_open(PING, 0)) == -1) {
		printf("failed to find ping, errno %d\n", errno );
		exit(1);
	}

	status = MsgSendPulse(ping_coid, PULSE_PRIORITY, PULSE_CODE, 1);
	if (-1 == status) {
		perror("MsgSendPulse");
		exit(EXIT_FAILURE);
	}

	while (1) {
		int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &info);
		if (rcvid == -1) {
			perror("MsgReceive");
			break;
		} else if (rcvid == 0) {
			mypulse = (struct _pulse *) &msg;
			printf("Pulse code: %d, pulse value %d\n", mypulse->code,
					mypulse->value.sival_int);
			sleep(1);
			status = MsgSendPulse(ping_coid, PULSE_PRIORITY, PULSE_CODE,
					mypulse->value.sival_int + 1);
		} else {
			printf("Unexpcted message");

		}
	}

	return EXIT_SUCCESS;
}
