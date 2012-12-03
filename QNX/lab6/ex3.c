#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <time.h>

#define TIME_PULSE_CODE 6
#define KEYBOARD_PULSE_CODE 7

struct sigevent event_time, event_keyboard;
int chid, coid;
int flag = 0;

const struct sigevent *handler_time( void *area, int id ) {
    // Wake up the thread every 100th interrupt
    if ( rand() % 10000 < 2 ) {
    	if(flag == 0) {
    		flag = 1;
    	} else {
    		flag = 0;
    	}
		return &event_time;
	}
    else {
        return( NULL );
    }
}

const struct sigevent *handler_keyboard( void *area, int id ) {
	return &event_keyboard;
}

int main3() {
    int id1, id2;
    struct _pulse pulse;

    int seed = time(NULL);
    srand(seed);

    // Request I/O privileges
    ThreadCtl( _NTO_TCTL_IO, 0 );

    chid = ChannelCreate( 0 );
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	SIGEV_PULSE_INIT( &event_time, coid, getprio(0), TIME_PULSE_CODE, 0 );
    id1=InterruptAttach( 0, &handler_time,
                        NULL, 0, 0 );

	SIGEV_PULSE_INIT( &event_keyboard, coid, getprio(0), KEYBOARD_PULSE_CODE, 0 );
    id2=InterruptAttach( 1, &handler_keyboard,
                        NULL, 0, 0 );

    for( ;; ) {
    	MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
    	if(pulse.code == TIME_PULSE_CODE) {
    		if(flag == 1) {
                printf( "Fire!\n" );
    		} else {
    			printf("No more shooting!\n");
    		}
    	} else if(pulse.code == KEYBOARD_PULSE_CODE) {
    		if(flag == 1) {
    			printf("\tYou are good :)\n");
    		} else {
    			printf("\tFalstart :(\n");
    		}
    	}
    	printf("got pulse with code %d\n\n", pulse.code);
    }

    // Disconnect the ISR handler
    InterruptDetach(id1);
    InterruptDetach(id2);
    return 0;
    }
