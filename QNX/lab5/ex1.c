/*
 * Demonstrate how to set up a timer that, on expiry,
 * sends us a pulse.  This example sets the first
 * expiry to 1.5 seconds and the repetition interval
 * to 1.5 seconds.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <string.h>

#define TIMER_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define PASSWORD_PULSE_CODE   _PULSE_CODE_MINAVAIL + 1

typedef union {
	struct _pulse pulse;
/* your other message structures would go
 here too */
} my_message_t;

void *read_password(void * _args) {
	int srv_coid = (int) _args;
	char password[] = {'a', 'b', 'c', 'd', '\0'};
	char read_password[5];
	do {
		printf("Password:\n");
		scanf("%s", read_password);
	} while(strcmp(password, read_password) != 0);

	int status = MsgSendPulse(srv_coid, getprio(0), PASSWORD_PULSE_CODE, getpid());
	if (-1 == status) {
		perror("MsgSendPulse");
		exit(EXIT_FAILURE);
	}
	return 0;
}

int main1() {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;
	int chid;
	int rcvid;
	int srv_coid;
	my_message_t msg;

	chid = ChannelCreate(0);

	event.sigev_notify = SIGEV_PULSE;
	srv_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	event.sigev_coid = srv_coid;
	event.sigev_priority = getprio(0);
	event.sigev_code = TIMER_PULSE_CODE;


	/*
	 * As of the timer_settime(), we will receive our pulse
	 * in 1.5 seconds (the itime.it_value) and every 1.5
	 * seconds thereafter (the itime.it_interval)
	 */

	for (;;) {
		timer_create(CLOCK_REALTIME, &event, &timer_id);

		itime.it_value.tv_sec = 5;
		itime.it_value.tv_nsec = 0;
		//itime.it_interval.tv_sec = 1;
		//itime.it_interval.tv_nsec = 500000000;
		timer_settime(timer_id, 0, &itime, NULL);

		int tid;
		pthread_create(&tid, NULL, read_password, (void *)srv_coid);
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) { /* we got a pulse */
			if (msg.pulse.code == TIMER_PULSE_CODE) {
				pthread_cancel(tid);
				timer_delete(timer_id);
				printf("\nPonow probe\n\n");
				continue;
			} else if(msg.pulse.code == PASSWORD_PULSE_CODE) {
				pthread_cancel(tid);
				timer_delete(timer_id);
				printf("\nDobrze!\n\n");
				return 0;
			}
		}
	}
}
