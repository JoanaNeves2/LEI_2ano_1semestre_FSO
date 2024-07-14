#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

#define NTRIES  100

void do_somethingFork(void) { 
    // TODO:
    pid_t pid =fork();
    int status;    

    if(pid==0){
        exit(0);
    } else if(pid>0){
        wait(&status);
    }else{
        perror("fork");
        exit(1);
    }

}

void *thread_funcion(void *arg){
    return NULL;
}

void do_somethingThread(void) { 
    // TODO:
    pthread_t thread;
    pthread_create(&thread, NULL, thread_funcion, NULL);
    pthread_join(thread, NULL);

}

int main(int argc, char *argv[]) {
    int i;
    long elapsed;
    struct timeval t1,t2;

    gettimeofday(&t1, NULL);
    for (i = 0;  i < NTRIES; i++)
        do_somethingFork();  // code to evaluate
    gettimeofday(&t2, NULL);
    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L
            + (t2.tv_usec - t1.tv_usec);
    printf("Elapsed time (Fork) = %6li us (%g us/call)\n",
            elapsed, (double)elapsed/NTRIES);

    gettimeofday(&t1, NULL);
    for (i = 0;  i < NTRIES; i++)
        do_somethingThread();  // code to evaluate
    gettimeofday(&t2, NULL);
    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L
            + (t2.tv_usec - t1.tv_usec);
    printf("Elapsed time (Thread) = %6li us (%g us/call)\n",
            elapsed, (double)elapsed/NTRIES);

    return 0;
}

