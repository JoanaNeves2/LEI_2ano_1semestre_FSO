#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define NTRIES  1000	


int do_something(void) { return 1; }

int main(int argc, char *argv[]) {

    //é necessário verificar a quantidade de argumentos recebidos
    //após o time??

    int i, p;
    long elapsed;
    struct timeval t1,t2;

    gettimeofday(&t1, NULL);
    for (i = 0;  i < NTRIES; i++){
	//printf("Writing");//fflush(stdout);
    	//p=getuid();
        p = do_something();  // code to evaluate
        }
    gettimeofday(&t2, NULL);
    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L
            + (t2.tv_usec - t1.tv_usec);
    //runcmd(argv);
    printf("Elapsed time = %li us (%g us/call)\n",
            elapsed, (double)elapsed/NTRIES);
    return 0;
}

