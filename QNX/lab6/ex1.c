#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#define MY_PULSE_CODE1 6
#define MY_PULSE_CODE2 7

struct sigevent event1, event2;
volatile unsigned counter1 = 0, counter2 = 0;
int chid, coid;

const struct sigevent *handler1( void *area, int id ) {
    // Wake up the thread every 100th interrupt
    if ( ++counter1 == 1000 ) {
        counter1 = 0;
        return( &event1 );
        }
    else {
        return( NULL );
    }
}

const struct sigevent *handler2( void *area, int id ) {
    // Wake up the thread every 250th interrupt
    if ( ++counter2 == 2500 ) {
        counter2 = 0;
        return( &event2 );
        }
    else {
        return( NULL );
    }
}

int main() {
    int i;
    int id1, id2;
    struct _pulse pulse;

    // Request I/O privileges
    ThreadCtl( _NTO_TCTL_IO, 0 );

    chid = ChannelCreate( 0 );
	coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );

	SIGEV_PULSE_INIT( &event1, coid, getprio(0), MY_PULSE_CODE1, 0 );
    id1=InterruptAttach( SYSPAGE_ENTRY(qtime)->intr, &handler1,
                        NULL, 0, 0 );

	SIGEV_PULSE_INIT( &event2, coid, getprio(0), MY_PULSE_CODE2, 0 );
    id2=InterruptAttach( SYSPAGE_ENTRY(qtime)->intr, &handler2,
                        NULL, 0, 0 );

    for( i = 0; i < 10; ++i ) {
        // Wait for ISR to wake us up
    	MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL );
    	if(pulse.code == MY_PULSE_CODE1) {
            printf( "1000 events\n" );
    	} else if(pulse.code == MY_PULSE_CODE2) {
    		printf( "2500 events\n" );
    	}
    }

    // Disconnect the ISR handler
    InterruptDetach(id1);
    InterruptDetach(id2);
    return 0;
}
