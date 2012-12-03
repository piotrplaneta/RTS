#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#define MY_PULSE_CODE 6

struct sigevent event;
volatile unsigned counter;
int chid, coid;

void f1() {
	printf("Function 1\n");
}

void f2() {
	printf("Function 2\n");
}

void f3() {
	printf("Function 3\n");
}

void f4() {
	printf("Function 4\n");
}

void f5() {
	printf("Function 5\n");
}

void f6() {
	printf("Function 6\n");
}

void (*functions[6])() = { &f1, &f2, &f3, &f4, &f5, &f6 };

const struct sigevent *clock_plannist(void *area, int id) {
	// Wake up the thread every 100th interrupt
	counter++;
	if (counter == 20000) {
		counter = 0;
	}
	if (counter == 1 || counter == 1000 || counter == 2000 || counter == 4000
			|| counter == 8000 || counter == 16000) {
		return (&event);
	} else {
		return (NULL);
	}
}

int main2() {
	int i = 0;
	int id;

	// Request I/O privileges
	ThreadCtl(_NTO_TCTL_IO, 0);

	// Initialize event structure
	event.sigev_notify = SIGEV_INTR;

	// Attach ISR vector
	id = InterruptAttach(SYSPAGE_ENTRY(qtime)->intr, &clock_plannist, NULL, 0, 0);

	for (;;) {
		// Wait for ISR to wake us up
		InterruptWait(0, NULL );
		functions[i++]();
		if (i == 6) {
			i = 0;
		}
	}

	// Disconnect the ISR handler
	InterruptDetach(id);
	return 0;
}
