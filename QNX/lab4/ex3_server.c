#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <ctype.h>

#include "msg_def.h"

int clients_count = 0;
list_node *head = NULL;

void calculate_reply(char *msg,char *buffer) {
    int i = 0;
    while(msg[i]) {
    	buffer[i] = tolower(msg[i]);
    	i++;
    }
    buffer[i]='\0';
}

void *broadcast_new_clients_count(void *scoid_not_to_broadcast) {
	int scoid = (int) scoid_not_to_broadcast;
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1 && current_node->scoid != scoid) {
			MsgDeliverEvent(current_node->rcvid, &current_node->event);
		}
		current_node = current_node->next;
	}
	return 0;
}

void print_list() {
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1) {
			printf("\tKlient o numerze %d, loginie %s\n", current_node->scoid, current_node->login);
		}

		current_node = current_node->next;
	}
}

void remove_node(int scoid) {
	list_node *current_node = head;
	list_node *previous_node = NULL;
	while(current_node != NULL) {
		if(current_node->scoid == scoid) {
			if(previous_node != NULL) {
				previous_node->next = current_node->next;
			}
			else {
				head = current_node->next;
			}
		}
		previous_node = current_node;
		current_node = current_node->next;
	}
	clients_count--;
	int tid;
	pthread_create(&tid, NULL, broadcast_new_clients_count, (void *)(-1));
}

void add_new_client(int rcvid, message msg, struct _msg_info info) {
	int status;
	char buffer[SIZE];
	clients_count++;
	printf("Message received from %d, %d\n",rcvid,info.pid);
	calculate_reply(msg.login, buffer);
	status = MsgReply(rcvid, EOK, &buffer, sizeof(buffer));
	if(-1 == status) perror("MsgReply");

	list_node *current_node = head;
	while(current_node->next != NULL) {
		current_node = current_node->next;
	}

	current_node->next = malloc (sizeof(list_node));
	current_node->next->scoid = info.scoid;
	current_node->next->rcvid = rcvid;
	current_node->next->event = msg.event;
	strcpy(current_node->next->login, msg.login);
	current_node->next->next = NULL;

	print_list();

	int tid;
	pthread_create(&tid, NULL, broadcast_new_clients_count, (void *)info.scoid);
}

int main(int argc, char **argv) {
	int rcvid, status;
	message msg;
	name_attach_t *attach;
	struct _msg_info info;
	struct _pulse *mypulse;

	/* attach the name the client will use to find us */
	/* our channel will be in the attach structure */
	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		printf("server:failed to attach name, errno %d\n", errno );
		exit(1);
	}

	head = malloc (sizeof(list_node));
	head->scoid = -1;
	head->next = NULL;

	while(1) {
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &info);
		if(rcvid == -1) {
			perror("MsgReceive");
			break;
		}
		else if (rcvid > 0) {
			if (msg.type == MSG_NEW_CLIENT) {
				add_new_client(rcvid, msg, info);
			} else if (msg.type == MSG_CLIENTS_COUNT) {
				msg.clients_count = clients_count;
				status = MsgReply(rcvid, EOK, &msg, sizeof(msg));
				if(-1 == status) perror("MsgReply");
			}
		}
		else {
			mypulse =(struct _pulse *)&msg;
			if (mypulse->code == _PULSE_CODE_DISCONNECT) {
				printf("Klient o numerze %d sie rozlaczyl\n", mypulse->scoid);
				remove_node(mypulse->scoid);
				print_list();
			}
		}
	}


	return 0;
}
