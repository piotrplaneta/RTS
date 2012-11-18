#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <unistd.h>
#include <sys/dispatch.h>

#include "msg_def.h"

int main1(int argc, char* argv[])
{
    int coid;         //Connection ID to server
    int status;       //status return value used for ConnectAttach and MsgSend

    mss_t msg;
    char incoming_buffer[SIZE];

    setvbuf (stdout, NULL, _IOLBF, 0);

    if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
		return EXIT_FAILURE;
	}

	msg.type = 1;
	strcpy(msg.text, argv[1]);
	printf("Wysylanie: %s\n", msg.text);
	status = MsgSend(coid, &msg, sizeof(msg), &incoming_buffer, sizeof(incoming_buffer) );
	if(-1 == status) {
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}
	printf("Odebrany bufor =%s z serwera\n", incoming_buffer);
	printf("MsgSend status: %d\n", status);

	sleep(10);
	name_close(coid);

    return EXIT_SUCCESS;
}
