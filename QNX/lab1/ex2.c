#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main2( void )
  {
    int rc;

    rc = system( "ps -l" );
    if( rc == -1 ) {
      printf( "shell could not be run\n" );
    } else {
      printf( "Result: %d\n",
          WEXITSTATUS( rc ) );
    }
    return EXIT_SUCCESS;
  }
