
/**
 * block_queue
 *
 * interface to a queue of data blocks
 * each block includes the data array and the size of data in this block
 * 
 * before use the queue must be initialized by calling queue_init
 *
 * (c) 2023 - Vitor Duarte FCT UNL
 */

#ifndef _BLOCKQUEUE
#define _BLOCKQUEUE

#define BLKSZ 1024  // data block max size

/** public functions **/

int queue_init( int size );

void queue_put(void *data, int size);

int queue_take(void *data);


#endif //_BLOCKQUEUE
