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

#define HYPERPERIOD_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define FRAME_PULSE_CODE   _PULSE_CODE_MINAVAIL + 1

typedef union {
	struct _pulse pulse;
/* your other message structures would go
 here too */
} my_message_t;

void function11() {
	delay(300);
	printf("one\n");
}

void function12() {
	delay(300);
	printf("two\n");

}

void function13() {
	delay(300);
	printf("three\n");
}

void function4() {
	delay(300);
	printf("four\n");
}

void *print_functions (void *args) {
	int i = (int) args;
	int j;

	if (i % 4 == 0) {
		for(j = 0; j < 3; j++) {
			function1();
		}
		for(j = 0; j < 3; j++) {
			function2();
		}
		for(j = 0; j < 3; j++) {
			function3();
		}
		for(j = 0; j < 3; j++) {
			function4();
		}
	}
	if (i % 4 == 1) {
		for(j = 0; j < 3; j++) {
			function2();
		}
		for(j = 0; j < 3; j++) {
			function3();
		}
		for(j = 0; j < 3; j++) {
			function4();
		}
		for(j = 0; j < 3; j++) {
			function1();
		}
	}
	if (i % 4 == 2) {
		for(j = 0; j < 3; j++) {
			function3();
		}
		for(j = 0; j < 3; j++) {
			function4();
		}
		for(j = 0; j < 3; j++) {
			function1();
		}
		for(j = 0; j < 3; j++) {
			function2();
		}
	}
	if (i % 4 == 3) {
		for(j = 0; j < 3; j++) {
			function4();
		}
		for(j = 0; j < 3; j++) {
			function1();
		}
		for(j = 0; j < 3; j++) {
			function2();
		}
		for(j = 0; j < 3; j++) {
			function3();
		}
	}
	return 0;
}

int main3(int argc, char **argv) {
	struct sigevent event_frame;
	struct itimerspec itime_frame;
	timer_t frame_timer_id;
	int chid;
	int rcvid;
	int srv_coid;
	my_message_t msg;

	if(argc != 3) {
		printf("proper usage: ./lab5 hyperperiod_duration_in_sec frames_count\n");
		return 0;
	}

	int duration = atoi(argv[1]);
	int frames_count = atoi(argv[2]);

	float frame_duration = (float)duration / (float)frames_count;

	int frame_duration_seconds = (int) frame_duration;
	int frame_duration_ns = (int) ((frame_duration - frame_duration_seconds) * 1000000000);

	chid = ChannelCreate(0);
	srv_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);


	event_frame.sigev_notify = SIGEV_PULSE;
	event_frame.sigev_coid = srv_coid;
	event_frame.sigev_priority = getprio(0);
	event_frame.sigev_code = FRAME_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event_frame, &frame_timer_id);
	itime_frame.it_value.tv_sec = frame_duration_seconds;
	itime_frame.it_value.tv_nsec = frame_duration_ns;
	itime_frame.it_interval.tv_sec = frame_duration_seconds;
	itime_frame.it_interval.tv_nsec = frame_duration_ns;
	timer_settime(frame_timer_id, 0, &itime_frame, NULL);


	int tid;
	int i = 0;
	pthread_create(&tid, NULL, print_functions, (void *)i);
	for (;;) {
		rcvid = MsgReceivePulse(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {
			if (msg.pulse.code == FRAME_PULSE_CODE) {
				i++;
				pthread_cancel(tid);
				pthread_create(&tid, NULL, print_functions, (void *)i);
				printf("\n\n\tKoniec ramki %d\n\n", i);
				if(i == frames_count) {
					printf("\n\n\t\tKoniec hiper-okresu\n\n");
					i = -1;
				}
			}
		}
	}
}
