#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/dispatch.h>

#include "msg_def.h"

void *receive_events2(void * _args) {
	int chid = (int) _args;
	struct _pulse pulse;
	int rcvid = MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL );
	printf("got pulse with code %d, was waiting for %d\n", pulse.code,
			MY_PULSE_CODE);
	return 0;
}

int main2(int argc, char **argv) {
	int chid, coid, srv_coid;
	struct my_msg msg;

	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate(0);

	/* and we need a connection to that channel for the pulse to be
	 delivered on */
	coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);

	/* fill in the event structure for a pulse */
	SIGEV_PULSE_INIT( &msg.event, coid, SIGEV_PULSE_PRIO_INHERIT,
			MY_PULSE_CODE, 0 );
	msg.type = MSG_GIVE_PULSE;

	/* find the server */
	if ((srv_coid = name_open(ATTACH_POINT, 0)) == -1) {
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	/* give the pulse event we initialized above to the server for
	 later delivery */
	MsgSend(srv_coid, &msg, sizeof(msg), NULL, 0);

	/* wait for the pulse from the server */
	int tid;
	pthread_create(&tid, NULL, receive_events2, (void *)chid);
	pthread_join(tid, NULL);
	return 0;
}
