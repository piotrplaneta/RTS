#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_

#define SIZE 100
#define PULSE_PRIORITY 10
#define PULSE_CODE 6
#define PING "ping"
#define PONG "pong"

typedef struct  {
   int  type;
   int  from;
   int  lowcase_count;
   char text[SIZE];
 } mss_t;

 #endif
