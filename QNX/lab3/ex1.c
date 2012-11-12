#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <ctype.h>
#include <errno.h>

typedef struct {
	int typ; //0 - koniec, 1 - nowy zakres, 10 - gotowosc
	int pocz; // poczatek zakresu
	int kon; // koniec zakresu
	int numer; // numer watku
} par_t;

volatile int running_threads = 0;
volatile int p_count = 0;
pthread_mutex_t primes_mutex = PTHREAD_MUTEX_INITIALIZER;
int pid;
int chid;

void *count_primes2(void *arg) {
	int status;
	int coid;
	par_t reply;

	coid = ConnectAttach(ND_LOCAL_NODE, pid, chid, _NTO_SIDE_CHANNEL, 0);
	while (1) {
		par_t args;
		args.typ = 10;
		args.numer = pthread_self();

		status = MsgSend(coid, &args, sizeof(args), &reply, sizeof(reply));
		if (reply.typ == 1) {
			sleep(1);

			int primes_count = 0;

			printf("Proces: %d, watek: %d\n", getpid(), pthread_self());
			int i;
			for (i = reply.pocz; i < reply.kon; i++) {
				int c;
				int prime = 1;
				for (c = 2; c <= i / 2; c++) {
					if (i % c == 0) {
						prime = 0;
						break;
					}
				}
				if (prime && i != 0 && i != 1)
					primes_count++;
			}

			//sleep(15 - args->numer);

			printf("\twatek: %d, poczatek %d, koniec %d, primes %d\n",
					pthread_self(), reply.pocz, reply.kon, primes_count);

			pthread_mutex_lock(&primes_mutex);
			p_count += primes_count;
			pthread_mutex_unlock(&primes_mutex);
		} else if (reply.typ == 0) {
			return 0;
		}

	}
}

int main(int argc, char *argv[]) {
	pid = getpid();
	chid = ChannelCreate(0);
	if (argc != 4) {
		printf("Proper usage: ./lab2 range_start range_end thread_count\n");
		return 0;
	}

	int range_start = atoi(argv[1]);
	int range_end = atoi(argv[2]);
	int threads_count = atoi(argv[3]);
	int range_length = (range_end - range_start) / (threads_count * 4);

	int i = 0;
	int f = 0;
	pthread_t tid;

	while (1) {

		if (running_threads < threads_count && !f) {
			pthread_create(&tid, NULL, count_primes2, NULL);
			running_threads++;
			continue;
		} else {
			f = 1;
			par_t args;
			int rcvid = MsgReceive(chid, &args, sizeof(args), NULL);
			if (range_start + (i + 1) * range_length <= range_end) {
				if (args.typ == 10) {
					args.typ = 1;
					args.numer = i;
					args.pocz = range_start + i * range_length;
					args.kon = range_start + (i + 1) * range_length;
					int status = MsgReply(rcvid, EOK, &args, sizeof(args));
					if (-1 == status) {
						perror("MsgReply");
					}
				}

				i++;
			} else {
				args.typ = 0;
				int status = MsgReply(rcvid, EOK, &args, sizeof(args));
				if (-1 == status) {
					perror("MsgReply");
				}
				printf("Watek %d poprosil, ale nie ma\n", args.numer);
				running_threads--;
				if (!running_threads) {
					break;
				}
			}
		}
	}
	printf("Liczb pierwszych: %d\n", p_count);

	return 0;
}
