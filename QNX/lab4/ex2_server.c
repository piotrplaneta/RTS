#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "msg_def.h"

void *send_event2(void * _args) {
	thread_args *args = (thread_args *) _args;
	/* wait until it is time to notify the client */
	sleep(3);

	/* deliver notification to client that client requested */

	MsgDeliverEvent(args->rcvid, &args->event);
	printf("server:delivered event\n");
	return 0;
}

int main2(int argc, char **argv) {
	int rcvid;
	struct my_msg msg;
	name_attach_t *attach;

	/* attach the name the client will use to find us */
	/* our channel will be in the attach structure */
	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		printf("server:failed to attach name, errno %d\n", errno );
		exit(1);
	}

	while(1) {
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL );
		MsgReply(rcvid, 0, NULL, 0);
		if (msg.type == MSG_GIVE_PULSE) {
			int tid;
			thread_args _args;
			_args.event = msg.event;
			_args.rcvid = rcvid;
			thread_args *args = &_args;
			pthread_create(&tid, NULL, send_event2, (void *)args);
		} else {
			printf("server: not-implemented message received \n");
		}
	}


	return 0;
}
