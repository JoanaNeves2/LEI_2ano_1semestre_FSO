
/**
 * stocks
 * 
 * the writer part of a shared memory demo. Just one can run.
 * this will be simulating the publishing of sotcks transactions on a
 * public board (in shared memory).
 * the readers-writer pattern should be used so that we don't write
 * when clients are reading.
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
#include <sys/mman.h>
#include <fcntl.h>

#include "shared_board.h"


#define LSIZE 1000



struct shared_board *board;   // the shared board!


sem_t *write_sem;
sem_t *reader_sem;

/**
 * create_board - creates shared memory with stocks board and extra variables
 *      needed. Map it to memory.
 *      (also create any other objects for concurrency control)
 */
void create_board() {
    // create and initialize semaphores
    write_sem = open("writer_semaphore", O_CREAT | O_EXCL, 0666, 1);
    if(write_sem == SEM_FAILED){
        perror("sem_open writer_semaphore");
        exit(1);
    }

    reader_sem = open("reader_semaphore", O_CREAT | O_EXCL, 0666, 1);
    if(reader_sem == SEM_FAILED){
        perror("sem_open reader_semaphore");
        exit(1);
    }

    // create and initialize shared memory
    //  don't forget ftruncate
    int fd = shm_open("/shared_board", O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        perror("shm_open");
        exit(1);
    }

    if(ftruncate(fd, sizeof(struct shared_board)) == -1){
        perror("ftruncate");
        exit(1);
    }

    board = mmap(NULL, sizeof(struct shared_boardd), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(board == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    //inicializar a placa compartilhada
    for(int i = 0; i < BOARD_SIZE; i++){
        strcpy(board->board[i].name, "");
        board->board[i].seq = 0;
        board->board[i].price = 0.0;
        board->board[i].amount = 0;
    }
   
}


/**
 * publish - adds a new transaction to the top of global board
 *      (must shift down table contents and clients should not see 
 *      inconsistent tables)
 */
void publish(int seq, char *comp, float price, int num) {
    // writer lock
    sem_wait(write_sem);

    for ( int i=9; i>0; i--) {  // shift down
       board->board[i] = board->board[i-1];
       usleep(1);               // slowlly so data races can easely appear
    }

    strcpy(board->board[0].name, comp);
    board->board[0].seq = seq;
    board->board[0].price = price;
    board->board[0].amount = num;

    // writer unlock
    sem_post(write_sem);
}


/**
 * main - starts by opening a transactions file that will be used to
 *      simulate the operations to put in the public shared board. 
 *      There is only one stocks running and this must be the "owner"
 *      of shared memory and concorrency control objects, so it is
 *      responsable for its creation and initializations.
 */
int main(int argc, char *argv[]) {
    char line[LSIZE];
    int count = 0;
    FILE *trace;
    if ( argc!=2 || (trace = fopen(argv[1], "r"))==NULL ) {
        if (argc==2) perror(argv[1]);
        fprintf(stderr,"usage: %s file\n", argv[0]);
        exit(1);
    }

    create_board();
    
    while ( fgets( line, LSIZE, trace )!=NULL ) { // simulate exchange operations
        char comp[5];
        float price;
        int amount;
        count++;
        sscanf(line, "%s%f%i", comp, &price, &amount);
        printf("adding:  %i: %s $%f %i\n", count, comp, price, amount);
        publish(count, comp, price, amount);
        printf("done.\n\n");
        usleep(200000);
    }
    return 0;
}
