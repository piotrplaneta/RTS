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

#define FUNCTION1_COID_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define FUNCTION2_COID_PULSE_CODE   _PULSE_CODE_MINAVAIL + 1
#define FUNCTION3_COID_PULSE_CODE   _PULSE_CODE_MINAVAIL + 2
#define FUNCTION1_GO_PULSE_CODE 	_PULSE_CODE_MINAVAIL + 3
#define FUNCTION2_GO_PULSE_CODE 	_PULSE_CODE_MINAVAIL + 4
#define FUNCTION3_GO_PULSE_CODE 	_PULSE_CODE_MINAVAIL + 5

typedef union {
	struct _pulse pulse;
/* your other message structures would go
 here too */
} my_message_t;

void *function1(void *args) { //T1 = 2 sec
	my_message_t msg;
	int srv_coid = (int) args;
	int my_chid = ChannelCreate(0);
	int my_coid = ConnectAttach(ND_LOCAL_NODE, 0, my_chid, _NTO_SIDE_CHANNEL, 0);
	MsgSendPulse(srv_coid, getprio(0), FUNCTION1_COID_PULSE_CODE, my_coid);
	while(1) {
		int rcvid = MsgReceivePulse(my_chid, &msg, sizeof(msg), NULL);
		printf("\tpulse received - function1\n");
		if (rcvid == 0) {
			if (msg.pulse.code == FUNCTION1_GO_PULSE_CODE) {
				int i;
				for (i = 0; i < 6; i++) {
					delay(300);
					printf("one\n");
				}
			}
		}
	}
}

void *function2(void *args) { //T2 = 5 sec
	my_message_t msg;
	int srv_coid = (int) args;
	int my_chid = ChannelCreate(0);
	int my_coid = ConnectAttach(ND_LOCAL_NODE, 0, my_chid, _NTO_SIDE_CHANNEL, 0);
	MsgSendPulse(srv_coid, getprio(0), FUNCTION2_COID_PULSE_CODE, my_coid);
	while(1) {
		int rcvid = MsgReceivePulse(my_chid, &msg, sizeof(msg), NULL);
		printf("\tpulse received - function2\n");
		if (rcvid == 0) {
			if (msg.pulse.code == FUNCTION2_GO_PULSE_CODE) {
				int i;
				for (i = 0; i < 15; i++) {
					delay(300);
					printf("two\n");
				}
			}
		}
	}
}

void *function3(void *args) { //T3 = 10 sec
	my_message_t msg;
	int srv_coid = (int) args;
	int my_chid = ChannelCreate(0);
	int my_coid = ConnectAttach(ND_LOCAL_NODE, 0, my_chid, _NTO_SIDE_CHANNEL, 0);
	MsgSendPulse(srv_coid, getprio(0), FUNCTION3_COID_PULSE_CODE, my_coid);
	while(1) {
		int rcvid = MsgReceivePulse(my_chid, &msg, sizeof(msg), NULL);
		printf("\tpulse received - function3\n");
		if (rcvid == 0) {
			if (msg.pulse.code == FUNCTION3_GO_PULSE_CODE) {
				int i;
				for (i = 0; i < 30; i++) {
					delay(300);
					printf("three\n");
				}
			}
		}
	}
}

void make_function1_timer(int coid) {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_priority = 10;
	event.sigev_code = FUNCTION1_GO_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);
	itime.it_value.tv_sec = 2;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 2;
	itime.it_interval.tv_nsec = 0;
	timer_settime(timer_id, 0, &itime, NULL);
}

void make_function2_timer(int coid) {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_priority = 5;
	event.sigev_code = FUNCTION2_GO_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);
	itime.it_value.tv_sec = 5;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 5;
	itime.it_interval.tv_nsec = 0;
	timer_settime(timer_id, 0, &itime, NULL);
}

void make_function3_timer(int coid) {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;
	event.sigev_priority = 2;
	event.sigev_code = FUNCTION3_GO_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);
	itime.it_value.tv_sec = 10;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 10;
	itime.it_interval.tv_nsec = 0;
	timer_settime(timer_id, 0, &itime, NULL);
}



int main(int argc, char **argv) {
	int chid;
	int rcvid;
	int srv_coid;
	my_message_t msg;
	int tid;

	chid = ChannelCreate(0);
	srv_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	int function1_coid;
	int function2_coid;
	int function3_coid;

	pthread_attr_t attr;
	struct sched_param params;

	pthread_attr_init( &attr );
	params.sched_priority = 10;
	pthread_attr_setschedparam(&attr, &params);
	pthread_create(&tid, &attr, function1, (void *)srv_coid);
	rcvid = MsgReceivePulse(chid, &msg, sizeof(msg), NULL);
	printf("pulse received\n");
	if (rcvid == 0) {
		if (msg.pulse.code == FUNCTION1_COID_PULSE_CODE) {
			function1_coid = msg.pulse.value.sival_int;
			printf("%d\n", function1_coid);
		}
	}

	pthread_attr_init( &attr );
	params.sched_priority = 5;
	pthread_attr_setschedparam(&attr, &params);
	pthread_create(&tid, &attr, function2, (void *)srv_coid);
	rcvid = MsgReceivePulse(chid, &msg, sizeof(msg), NULL);
	printf("pulse received\n");
	if (rcvid == 0) {
		if (msg.pulse.code == FUNCTION2_COID_PULSE_CODE) {
			function2_coid = msg.pulse.value.sival_int;
			printf("%d\n", function2_coid);
		}
	}

	pthread_attr_init( &attr );
	params.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &params);
	pthread_create(&tid, &attr, function3, (void *)srv_coid);
	rcvid = MsgReceivePulse(chid, &msg, sizeof(msg), NULL);
	printf("pulse received\n");
	if (rcvid == 0) {
		if (msg.pulse.code == FUNCTION3_COID_PULSE_CODE) {
			function3_coid = msg.pulse.value.sival_int;
			printf("%d\n", function3_coid);
		}
	}



	pthread_attr_destroy(&attr);

	make_function1_timer(function1_coid);
	make_function2_timer(function2_coid);
	make_function3_timer(function3_coid);

	pthread_join(tid, NULL);
	return 0;
}
