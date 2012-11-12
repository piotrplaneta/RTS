#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>     // #define for ND_LOCAL_NODE is in here
#include <process.h>

#include "msg_def.h"

int main4(int argc, char* argv[]) {
	int coid; //Connection ID to server
	int status; //status return value used for ConnectAttach and MsgSend
	mms reply;
	int file;
	int type;

	if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
		return EXIT_FAILURE;
	}

	while (1) {
		scanf("%d", &type);
		if (type == OPENR) {
			mms msg;
			printf("Nazwa pliku:\n");
			scanf("%s", msg.buffer);
			msg.type = OPENR;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			if(-1 == status) {
				perror("MsgSend");
				exit(EXIT_FAILURE);
			}
			file = reply.fh;
			printf("Otwarto plik, deskryptor: %d\n", file);
			continue;
		}
		if (type == OPENW) {
			mms msg;
			printf("Nazwa pliku:\n");
			scanf("%s", msg.buffer);
			msg.type = OPENW;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			if(-1 == status) {
				perror("MsgSend");
				exit(EXIT_FAILURE);
			}
			file = reply.fh;
			printf("Otwarto plik, deskryptor: %d\n", file);
			continue;
		}
		if (type == READ) {
			mms msg;
			printf("Ilosc bajtow:\n");
			scanf("%d", &(msg.bytes_count));
			msg.fh = file;
			msg.type = READ;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			printf("Odczytano %d bajtow, tresc: %s\n", reply.bytes_count, reply.buffer);
			continue;
		}
		if (type == WRITE) {
			mms msg;
			printf("Tresc:\n");
			scanf("%s", msg.buffer);
			msg.fh = file;
			msg.type = WRITE;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			printf("Zapisano %d bajtow, tresc: %s\n", reply.bytes_count, reply.buffer);
			continue;
		}
		if (type == CLOSE) {
			mms msg;
			msg.fh = file;
			msg.type = CLOSE;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			printf("Zamknieto plik o deskryptorze %d, rezultat %d\n", reply.fh, reply.buffer);
			continue;
		}
		if (type == STOP) {
			mms msg;
			msg.type = STOP;
			status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply) );
			printf("Koniec\n");
			break;
		}
	}

	return EXIT_SUCCESS;
}

