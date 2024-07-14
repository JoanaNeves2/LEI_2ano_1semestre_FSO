
/**
 * client
 * 
 * the reader part of a shared memory demo.
 * this will be continuous reading the data in a shared memory.
 * the readers-writer pattern should be used so that we don't see
 * the data in the middle of an update by the writer program, the stocks.
 * more than one client can be running and the socks must be running first.
 * 
 * 2023 - Vitor Duarte
 * DI - FCT UNL
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "shared_board.h"



struct shared_board *board;   // the shared board


sem_t *reader_sem;

/**
 * check_board - check for global board consistency 
 *      only checks if all updates are present and if there are no repetitions
 * returns 0 if errors are faund
 */
int check_board() {
    int s=board->board[0].seq;
    int err=0;
    
    for (int i=1; i<10; i++) {
        if (s-1 != board->board[i].seq) {   // transactions out of order
            printf(" (%i -> %i) ", s, board->board[i].seq);
            err++;
        }
        s = board->board[i].seq;
    }
    return !err;
}


/**
 * print_board - clears screen and prints global board content
 *      also prints error messages if board is inconsistent
 *      (aka in the middle of an updade)
 */
void print_board() {
    printf("\033c\n");    // clear vt100 compatible terminal
    
    // readers lock
    sem_wait(reader_sem);

    if (!check_board()) {
        printf("-----  ERROR\n");
        usleep(500000);   // give time for us to see the message
    }
    for (int i=0; i<10; i++) {
        printf("%i: %s %f %i\n", board->board[i].seq, board->board[i].name, 
                board->board[i].price, board->board[i].amount);
    }
    
    // readers unlock
    sem_post(reader_sem);
}


/**
 * get_board - open shared memory with stocks board and map it to memory
 *      (also get access to other objects for concurrency control)
 */
void get_board() {
    // TODO:
    // get semaphores
    reader_sem = sem_open("reader_semaphore", 0);
    if(reader_sem == SEM_FAILED){
        perror("sem_open reader_semaphore");
        exit(1);
    }

    // get shared memory
    int fd = shm_open("/shared_board", O_RDWR, 0666);
    if(fd == -1){
        perror("shm_open");
        exit(1);
    }
    
    //board = mmap(...)
    board = mmap(NULL, sizeof(struct shared_boardd), PROT_READ, MAP_SHARED, fd, 0);
    if(board == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
   
}



/**
 * main - run forever printing the content of the shared
 *      global stocks transactions board
 */
int main() {
    get_board();
    
    while (1) {
        print_board();  // busy waiting, but just for this demo ...
        usleep(100000);
    }
    
    sem_close(reader_sem);

    return 0;
    
}
