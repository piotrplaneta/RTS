#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>     // #define for ND_LOCAL_NODE is in here
#include <process.h>
#include <sys/wait.h>

#include "msg_def.h"

int main3(int argc, char* argv[])
{
	int coid;         //Connection ID to server
	int status;       //status return value used for ConnectAttach and MsgSend
	int server_pid;   //server's process ID
	int server_chid;  //server's channel ID
	kom_t msg;
	kom_t reply;

	if(3 != argc) {
		printf("This program must be started with commandline arguments, for example:\n\n");
		printf("   cli 482834 1   \n\n");
		printf(" 1st arg(482834): server's pid\n");
		printf(" 2nd arg(1): server's chid\n");
		exit(EXIT_FAILURE);
	}

	server_pid = atoi(argv[1]);
	server_chid = atoi(argv[2]);

	printf("attempting to establish connection with server pid: %d, chid %d\n", server_pid, server_chid);

	coid = ConnectAttach(ND_LOCAL_NODE, server_pid, server_chid, _NTO_SIDE_CHANNEL, 0);
	if(-1 == coid) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	int i = 0;
	while(i++ < 10) {
		msg.type = 1;
		msg.from = getpid();
		sprintf(msg.text, "Proces %d, krok petli %d\n", getpid(), i);

		status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
		if(-1 == status) {
			perror("MsgSend");
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}

	msg.type = 0;
	status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );


	exit(EXIT_SUCCESS);
}

