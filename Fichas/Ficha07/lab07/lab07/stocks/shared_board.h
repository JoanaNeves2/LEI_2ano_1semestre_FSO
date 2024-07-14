
/**
 * shared_board.h
 * 
 * global definitions shared by stocks (the writer) and
 * client (the readers)
 *
 * 2023 Vitor Duarte
 * DI - FCT UNL
 */
 

#define WRLOCK  "/stockwriterlock"
#define MUTEX   "/stockmutex"
#define SHMEM   "/stockshmem"


typedef struct {
    int seq;       // sequence number
    char name[5];  // company name 4 char+\0
    float price;   // socks price
    int amount;    // number of stocks exchanged
} stock_entry;

struct shared_board {
    stock_entry board[10]; // last 10 transactions
    //int check;             // check number // esta linha não é necessária
    int readers_count;
};
