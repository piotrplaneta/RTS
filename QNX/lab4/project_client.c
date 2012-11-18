#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/dispatch.h>
#include <string.h>

#include "msg_def.h"

int srv_coid;
char login[SIZE];

void *receive_pulses(void * _args) {
	message msg, reply;
	while(1) {
		int chid = (int) _args;
		struct _pulse pulse;
		MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL );
		if(pulse.code == NEW_MESSAGE_PULSE_CODE) {
			msg.type = MSG_GET_MESSAGE;
			strcpy(msg.login, login);
			MsgSend(srv_coid, &msg, sizeof(msg), &reply, sizeof(reply));
			if (strcmp(reply.message_from, "Server0") != 0) {
				printf("\t[Message] %s: %s\n", reply.message_from, reply.message);
			}
			else {
				printf("\t%s\n", reply.message);
			}
		} else if(pulse.code == NEW_CLIENTS_COUNT_PULSE_CODE) {
			msg.type = MSG_CLIENTS_COUNT;
			MsgSend(srv_coid, &msg, sizeof(msg), &reply, sizeof(reply));
			printf("\t[Clients count] %d\n", reply.clients_count);
		}
	}

	return 0;
}

int main(int argc, char **argv) {
	int chid, coid;
	struct my_msg msg_to_get_clients_count;
	message msg_to_get_messages;
	char command[SIZE];
	if(argc != 2) {
		printf("Proper usage: ./project_client login\n");
		return 0;
	}

	strcpy(login, argv[1]);

	/* we need a channel to receive the pulse notification on */
	chid = ChannelCreate(0);

	/* and we need a connection to that channel for the pulse to be
	 delivered on */
	coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);

	if ((srv_coid = name_open(ATTACH_POINT, 0)) == -1) {
		printf("failed to find server, errno %d\n", errno );
		exit(1);
	}

	SIGEV_PULSE_INIT( &msg_to_get_clients_count.event, coid, SIGEV_PULSE_PRIO_INHERIT,
			NEW_CLIENTS_COUNT_PULSE_CODE, 0 );
	msg_to_get_clients_count.type = MSG_NEW_CLIENT;
	strcpy(msg_to_get_clients_count.login, argv[1]);
	MsgSend(srv_coid, &msg_to_get_clients_count, sizeof(msg_to_get_messages), NULL, 0);

	SIGEV_PULSE_INIT( &msg_to_get_messages.event, coid, SIGEV_PULSE_PRIO_INHERIT,
			NEW_MESSAGE_PULSE_CODE, 0 );
	msg_to_get_messages.type = MSG_NEW_CLIENT_GET_MESSAGES;
	strcpy(msg_to_get_messages.login, login);
	MsgSend(srv_coid, &msg_to_get_messages, sizeof(msg_to_get_messages), NULL, 0);

	int tid;
	pthread_create(&tid, NULL, receive_pulses, (void *)chid);

	while(1) {
		printf("Type command (list - list of users, message - new message)\n");

		scanf("%s", command);

		if(strcmp(command, "list") == 0) {
			message msg_to_get_users;
			logins list;
			msg_to_get_users.type = MSG_GET_CLIENTS_LIST;
			MsgSend(srv_coid, &msg_to_get_users, sizeof(msg_to_get_users), &list, sizeof(list));
			printf("==LIST OF LOGINS: ==\n\n");
			printf("%s", list.list_of_logins);
			printf("\n==END OF LIST==\n");

		} else if(strcmp(command, "message") == 0) {
			message msg_to_send;
			msg_to_send.type = MSG_MESSAGE;
			printf("User login:\n");
			scanf("%s", msg_to_send.login);
			printf("Message:\n");
			scanf("%s", msg_to_send.message);
			strcpy(msg_to_send.message_from, login);
			MsgSend(srv_coid, &msg_to_send, sizeof(msg_to_send), NULL, 0);
		}
	}
	return 0;
}
