#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_

#define SIZE 100
#define PULSE_PRIORITY 10
#define PULSE_CODE 6
#define ATTACH_POINT "comunicator"

typedef struct  {
   int  type;
   int  from;
   int  lowcase_count;
   char text[SIZE];
 } mss_t;

typedef struct listNode {
         struct listNode* next;
         int scoid;
         int rcvid_clients_count;
         int rcvid_new_message;
         char login[SIZE];
         struct sigevent clients_count_event;
         struct sigevent new_message_event;
         char buffer[SIZE];
         char message_from[SIZE];
} list_node;

typedef struct my_msg
{
   short type;
   char login[SIZE];
   char message[SIZE];
   char message_from[SIZE];
   struct sigevent event;
   int clients_count;
} message;


typedef struct {
	int rcvid;
	struct sigevent event;
} thread_args;

typedef struct {
	char list_of_logins[1024];
} logins;


#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL+5
#define MSG_GIVE_PULSE _IO_MAX+4
#define NEW_CLIENTS_COUNT_PULSE_CODE 122
#define MSG_NEW_CLIENT 123
#define MSG_CLIENTS_COUNT 124
#define MSG_GET_CLIENTS_LIST 125
#define MSG_SEND_MESSAGE 126
#define NEW_MESSAGE_PULSE_CODE 127
#define MSG_MESSAGE 128
#define MSG_NEW_CLIENT_GET_MESSAGES 129
#define MSG_GET_MESSAGE 130

#endif
