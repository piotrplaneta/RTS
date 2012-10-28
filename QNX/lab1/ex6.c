#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <process.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>


void signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);
   // Cleanup and close up stuff here

   // Terminate program
   exit(signum);
}

int main6(int argc, char *argv[]) {

	// Define and object of structure
	   // rlimit.
	   struct rlimit rl;

	   // First get the time limit on CPU
	   getrlimit (RLIMIT_CPU, &rl);

	   printf("\n Default value is : %lld\n", (long long int)rl.rlim_cur);

	   // Change the time limit
	   rl.rlim_cur = 1;

	   // Now call setrlimit() to set the
	   // changed value.
	   setrlimit (RLIMIT_CPU, &rl);

	   // Again get the limit and check
	   getrlimit (RLIMIT_CPU, &rl);

	   printf("\n Default value now is : %lld\n", (long long int)rl.rlim_cur);

	   signal(SIGXCPU, signal_callback_handler);


	   // Simulate a long time consuming work
	   while (1);

	   return 0;

}
