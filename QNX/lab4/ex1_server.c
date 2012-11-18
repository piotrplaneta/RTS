#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <ctype.h>
#include <string.h>
#include <sys/dispatch.h>

#include "msg_def.h"

list_node *head1 = NULL;

void calculate_reply1(char *msg,char *buffer) {
    int i = 0;
    while(msg[i]) {
    	buffer[i] = tolower(msg[i]);
    	i++;
    }
    buffer[i]='\0';
}
void print_list1() {
	list_node *current_node = head1;
	while(current_node != NULL) {
		if(current_node->scoid != -1) {
			printf("\tKlient o numerze %d, loginie %s\n", current_node->scoid, current_node->login);
		}

		current_node = current_node->next;
	}
}

void remove_node1(int scoid) {
	list_node *current_node = head1;
	list_node *previous_node = NULL;
	while(current_node != NULL) {
		if(current_node->scoid == scoid) {
			if(previous_node != NULL) {
				previous_node->next = current_node->next;
			}
			else {
				head1 = current_node->next;
			}
		}
		previous_node = current_node;
		current_node = current_node->next;
	}
}

int main1(void) {
    int rcvid;
    int status;
    mss_t msg;
    struct _pulse *mypulse;
    struct _msg_info info;
    name_attach_t *attach;


    char buffer[SIZE];

    setvbuf (stdout, NULL, _IOLBF, 0);   //set IO to stdout to be line buffered

    if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		return EXIT_FAILURE;
	}

    head1 = malloc (sizeof(list_node));
	head1->scoid = -1;
	head1->next = NULL;

    while(1) {
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &info);
        if(rcvid == -1) {
            perror("MsgReceive");
            break;
        }
        else if (rcvid > 0) {
            printf("Message received from %d, %d\n",rcvid,info.pid);
            calculate_reply1(msg.text, buffer);
            status = MsgReply(rcvid, EOK, &buffer, sizeof(buffer));
            list_node *current_node = head1;
            while(current_node->next != NULL) {
            	current_node = current_node->next;
            }
            current_node->next = malloc (sizeof(list_node));
            current_node->next->scoid = info.scoid;
            strcpy(current_node->next->login, msg.text);
            current_node->next->next = NULL;
            print_list1();
            if(-1 == status) perror("MsgReply");
        }
        else {
            mypulse =(struct _pulse *)&msg;
            if (mypulse->code == _PULSE_CODE_DISCONNECT) {
            	printf("Klient o numerze %d sie rozlaczyl\n", mypulse->scoid);
            	remove_node1(mypulse->scoid);
            	print_list1();
            }
        }
    }

    return 0;
}
