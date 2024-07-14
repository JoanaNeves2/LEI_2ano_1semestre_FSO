/**
 * block_queue
 *
 * implementation of a queue of data blocks
 * each block includes the data array and the size of data in this block
 * 
 * before use the queue must be initialized by calling queue_init
 *
 * (c) 2023 - Vitor Duarte FCT UNL
 */

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>

#include "blockqueue.h"


// block of data
typedef struct {
    int size;          // used bytes in data array
    char data[BLKSZ];  // data
} block_t;


block_t *buffer;  // circular buffer 
int maxsize;      // circular buffer size
int used=0;           // buffer slots in use
int pidx=0, gidx= 0;  // put index and get index

sem_t empty;
sem_t full;
sem_t m;

#define FULLBUF (used==maxsize)
#define EMPTYBUF (used==0)


void putInBuffer(char *data, int s) {
    assert(used<maxsize);

    memcpy(&(buffer[pidx].data), data, s);
    buffer[pidx].size=s;
    pidx= (pidx+1)%maxsize;
    used++; 
}

int getFromBuffer(char *data) {
    assert(used>0);

    memcpy(data, &(buffer[gidx].data), buffer[gidx].size);
    int tmp=buffer[gidx].size;
    gidx = (gidx+1)%maxsize;
    used--;
    return tmp;
}

/** public functions **/

/** public functions **/

/**
 * queue_init - initialize queue of data blocks
 * size - max number of blocks
 * return 0 if failed to allocate queue
 */
int queue_init( int size ) {
    buffer = malloc( size*sizeof(block_t) );
    maxsize = size;
    sem_init(&m, 0, 1);
    sem_init(&empty, 0, size);
    sem_init(&full, 0, 0);
    return buffer != NULL;
}


/**
 * queue_put - put one block of data in queue
 * data - pointer to array of bytes
 * sz - array size
 */
void queue_put(void *data, int sz) {  // TODO:
    sem_wait(&empty);
    sem_wait(&m);
    putInBuffer(data, sz);
    sem_post(&m);
    sem_post(&full);
}


/**
 * queue_take - get next data block from queue
 * data - pointer to array (at least 1K) where to place the data
 * returns number of bytes placed in data
 */
int queue_take(void *data) {  // TODO:
    sem_wait(&full);
    sem_wait(&m);
    while (EMPTYBUF) {
    }
    int tmp = getFromBuffer(data);
    sem_post(&m);
    sem_post(&empty);
    return tmp;
}