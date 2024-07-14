#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>

#define SIZE 256*1024*1024
int *array;
int length, count;

int length_per_thread; // array slice size per thread

int *thread_count;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

// Code executed by each thread
//lab04 - without mutex neither conditional variables
/*void *count3s_thread(void *id){
    // TODO
    int index = (int)(intptr_t)id;
    int start = index*length_per_thread;
    int end = start +length_per_thread;    
    
    for (int i=start; i < end; i++){
        if (array[i] == 3){
            thread_count[index]++;
        }
    }    
}*/

//lab05 - with mutex and conditional variables
void *count3s_thread(void *id){
    int start = (int)(intptr_t) id * length_per_thread;

    for(int i=0;i<length_per_thread;i++){
        if(array[start+i] == 3){
            pthread_mutex_lock(&m);
            count++;
            pthread_mutex_unlock(&m);
        }
    }

}


// Code with the creation of multiple threads
void count3s(int nthreads){
    pthread_t tids[nthreads];
    count = 0;
    // TODO
	for(int i=0; i<nthreads;i++){
		pthread_create(&tids[i],NULL,count3s_thread,(void*)(intptr_t)i); 
	}

	for(int i=0;i<nthreads;i++){
		pthread_join(tids[i],NULL);
        //count += thread_count[i];
	}
}

int main( int argc, char *argv[]){

    if ( argc != 2 ) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]); //functions thread-safe and async-cancel-safe -> converts the initial portion  of the string pointes to by str to int representation
    if ( (SIZE % n) != 0 ) {
        printf("%d must be divisible by <number of threads>\n", SIZE);
        return 1;
    }

    array= (int *)malloc(SIZE*sizeof(int));
    length = SIZE;
    length_per_thread = SIZE / n;
    printf("Using %d threads; length_per_thread =  %d\n", n, length_per_thread);

    thread_count = (int *)calloc(n , sizeof(int)); //allocar memorio e colocar a 0 em vez de ser a NULL

    srand(0);  // always the same sequence
    for (int i=0; i < length; i++){
        array[i] = rand() % 4;
    }
    count3s(n);
    printf("Count of 3s = %d\n", count);
    return 0;
}

