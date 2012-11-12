#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <ctype.h>
#include <sys/dispatch.h>
#include <string.h>

#include "msg_def.h"

int main4(void) {
	name_attach_t *attach;
	mms msg;
	int rcvid;
	int result;

	/* Create a local name (/dev/name/local/...) */
	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		return EXIT_FAILURE;
	}

	/* Do your MsgReceive's here now with the chid */
	while (1) {
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) {/* Error condition, exit */
			break;
		}

		if (rcvid == 0) {/* Pulse received */
			printf("\tPulse received");
			continue;
		}

		if (msg.type == OPENR) {
			FILE *fp;
			fp = fopen(msg.buffer, "r");
			rewind(fp);
			msg.fh = fp;
			MsgReply(rcvid, EOK, &msg, sizeof(msg));
			continue;
		}
		if (msg.type == OPENW) {
			FILE *fp;
			fp = fopen(msg.buffer, "w");
			msg.fh = fp;
			MsgReply(rcvid, EOK, &msg, sizeof(msg));
			continue;
		}
		if (msg.type == READ) {
			FILE *fp = msg.fh;
			mms reply;
			result = fread(reply.buffer, sizeof(char), msg.bytes_count, fp);
			reply.bytes_count = result;
			reply.type = READ;
			reply.buffer[result] = '\0';
			msg.bytes_count = result;
			MsgReply(rcvid, EOK, &reply, sizeof(reply));
			continue;
		}
		if (msg.type == WRITE) {
			FILE *fp = msg.fh;
			fwrite (msg.buffer , sizeof(char), strlen(msg.buffer) , fp );
			msg.bytes_count = strlen(msg.buffer);
			MsgReply(rcvid, EOK, &msg, sizeof(msg));
			continue;
		}
		if (msg.type == CLOSE) {
			FILE *fp = msg.fh;
			result = fclose(fp);
			msg.fh = fp;
			if(result == 0) {
				strcpy(msg.buffer, "ok");
			} else {
				strcpy(msg.buffer, "not ok");
			}

			MsgReply(rcvid, EOK, &msg, sizeof(msg));
			continue;
		}
		if (msg.type == STOP) {
			MsgReply(rcvid, EOK, &msg, sizeof(msg));
			break;
		}

	}

	/* Remove the name from the space */
	name_detach(attach, 0);

	return EXIT_SUCCESS;
}

