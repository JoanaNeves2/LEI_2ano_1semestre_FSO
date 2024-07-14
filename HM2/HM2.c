#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int lenght, num, nrThreads;
char *id_threads;
pthread_t *tids;
pthread_mutex_t exmut;

int length_per_thread;


// Code executed by each thread
void *counts_thread(void *id){

    for(int i=0; i<=num; i++){
        pthread_mutex_lock( &exmut );
        printf("hello from %c: %i\n", *((char*) id), i);
        pthread_mutex_unlock( &exmut );
    }    
}

// Code with the creation of multiple threads
void counts(int nthreads, int finalNum){
    
    tids = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    id_threads = (char *)malloc(nthreads * sizeof(char));
    
    tids[nthreads];
    id_threads[nthreads];//incrementar em cada 
    char arg;
    // TODO

    for(int i=0; i<nrThreads;i++){
        id_threads[i] = 'A' +i;
    }

	for(int i=0; i<nthreads;i++){
		pthread_create(&tids[i],NULL,counts_thread,&id_threads[i]); 
	}

	for(int i=0;i<nthreads;i++){
		pthread_join(tids[i],NULL);
        //num++;
	}
	/*
	free(id_threads);
	free(tids);
	*/
}

int main( int argc, char *argv[]){

    nrThreads = atoi(argv[1]); //nº of threads tha user wants
    num = atoi(argv[2]); // num is the final num that user wants to count
    
    pthread_mutex_init( &exmut, NULL);

    counts(nrThreads,num);

    return 0;
}