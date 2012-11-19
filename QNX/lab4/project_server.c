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
			MsgDeliverEvent(current_node->rcvid_clients_count, &current_node->clients_count_event);
		}
		current_node = current_node->next;
	}
	return 0;
}

void *broadcast_disconnected_login(void *args) {

	char *disconnected_login = (char *) args;
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1) {
			strcpy(current_node->buffer, "[Disconnected] ");
			strcat(current_node->buffer, disconnected_login);
			strcpy(current_node->message_from, "Server0");
			MsgDeliverEvent(current_node->rcvid_new_message, &current_node->new_message_event);
		}
		current_node = current_node->next;
	}
	return 0;
}

void *broadcast_connected_login(void *args) {

	char *connected_login = (char *) args;
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1 && strcmp(connected_login, current_node->login) != 0) {
			strcpy(current_node->buffer, "[Connected] ");
			strcat(current_node->buffer, connected_login);
			strcpy(current_node->message_from, "Server0");
			MsgDeliverEvent(current_node->rcvid_new_message, &current_node->new_message_event);
		}
		current_node = current_node->next;
	}
	return 0;
}

void print_list() {
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1) {
			printf("\tClient %d, login %s\n", current_node->scoid, current_node->login);
		}

		current_node = current_node->next;
	}
}

void get_logins(char *buffer) {
	list_node *current_node = head;
	buffer[0] = 0;
	while(current_node != NULL) {
		if(current_node->scoid != -1) {
			strcat(buffer, current_node->login);
			strcat(buffer, "\n");
		}

		current_node = current_node->next;
	}
}

void remove_node(int scoid) {
	list_node *current_node = head;
	list_node *previous_node = NULL;
	char disconnected_login[SIZE];
	while(current_node != NULL) {
		if(current_node->scoid == scoid) {
			strcpy(disconnected_login, current_node->login);
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
	pthread_create(&tid, NULL, broadcast_disconnected_login, (void *) disconnected_login);
}

void add_new_client(int rcvid, message msg, struct _msg_info info) {
	clients_count++;
	printf("New client %d\n", info.pid);

	list_node *current_node = head;
	while(current_node->next != NULL) {
		current_node = current_node->next;
	}

	current_node->next = malloc (sizeof(list_node));
	current_node->next->scoid = info.scoid;
	current_node->next->rcvid_clients_count = rcvid;
	current_node->next->clients_count_event = msg.event;
	strcpy(current_node->next->login, msg.login);
	current_node->next->next = NULL;

	print_list();

	int tid;
	pthread_create(&tid, NULL, broadcast_new_clients_count, (void *)info.scoid);
	pthread_create(&tid, NULL, broadcast_connected_login, (void *)msg.login);
}

void add_messaging_data(int rcvid, message msg, struct _msg_info info) {
	list_node *current_node = head;
	while(current_node != NULL) {
		if (current_node->scoid != -1 && strcmp(current_node->login, msg.login) == 0) {
			current_node->rcvid_new_message = rcvid;
			current_node->new_message_event = msg.event;
		}
		current_node = current_node->next;
	}
}


void cache_message(message msg) {
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1 && strcmp(current_node->login, msg.login) == 0) {
			strcpy(current_node->buffer, msg.message);
			strcpy(current_node->message_from, msg.message_from);
			MsgDeliverEvent(current_node->rcvid_new_message, &current_node->new_message_event);
		}
		current_node = current_node->next;
	}
}

void deliver_message(int rcvid, message msg) {
	list_node *current_node = head;
	while(current_node != NULL) {
		if(current_node->scoid != -1 && strcmp(current_node->login, msg.login) == 0) {
			strcpy(msg.message, current_node->buffer);
			strcpy(msg.message_from, current_node->message_from);
			int status = MsgReply(rcvid, EOK, &msg, sizeof(msg));
			if(-1 == status) perror("MsgReply");
		}
		current_node = current_node->next;
	}
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
				status = MsgReply(rcvid, EOK, NULL, 0);
				if(-1 == status) perror("MsgReply");
			} else if (msg.type == MSG_NEW_CLIENT_GET_MESSAGES) {
				add_messaging_data(rcvid, msg, info);
				status = MsgReply(rcvid, EOK, NULL, 0);
				if(-1 == status) perror("MsgReply");
			} else if (msg.type == MSG_CLIENTS_COUNT) {
				msg.clients_count = clients_count;
				status = MsgReply(rcvid, EOK, &msg, sizeof(msg));
				if(-1 == status) perror("MsgReply");
			}  else if (msg.type == MSG_MESSAGE) {
				cache_message(msg);
				status = MsgReply(rcvid, EOK, NULL, 0);
				if(-1 == status) perror("MsgReply");
			} else if (msg.type ==MSG_GET_MESSAGE) {
				deliver_message(rcvid, msg);
			} else if (msg.type == MSG_GET_CLIENTS_LIST) {
				logins list;
				get_logins(list.list_of_logins);
				status = MsgReply(rcvid, EOK, &list, sizeof(list));
				if(-1 == status) perror("MsgReply");
			}
		}
		else {
			mypulse =(struct _pulse *)&msg;
			if (mypulse->code == _PULSE_CODE_DISCONNECT) {
				printf("Client %d has disconnected\n", mypulse->scoid);
				remove_node(mypulse->scoid);
				print_list();
			}
		}
	}


	return 0;
}
