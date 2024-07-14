/************************************************************************
 * FSO 23/24
 * 
 * copiar ficheiros usando stdio: fopen, fread, fwrite...
 * (versao multithreaded)
 * 
 * (c) 2017-2023 DI-FCT/UNL - Vitor Duarte
 ***********************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "blockqueue.h"

#define NBLK  5    // max number of blocks in queue

/**
 * terminate - exit program with error message
 */
void terminate( char *msg ) {
    if (msg != NULL) fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}


/**
 * thread reader - reads from filename passed in arg
 *          and temporarily puts in the queue
 */
void *reader(void*arg) {
    FILE* fin;
    int n;
    char buf[BLKSZ];

    //printf("reading %s\n", (char*)arg );
    fin = fopen( arg, "r" );
    if ( fin == NULL ) terminate( "opening input file" );
    while( (n= fread(buf, 1, BLKSZ, fin )) > 0 ) {
        queue_put( buf, n );
    }
    if ( n < 0 ) terminate( "read" );
    queue_put("", 0);	// we need to send a special block. WHY?
    fclose(fin);
    //printf("reading done\n" );  
    return NULL;
}

/**
 * thread writer - writes to filename passed in arg
 *          the data in the queue
 */
void *writer(void*arg) {
    FILE * fout;
    int nw, n;
    char buf[BLKSZ];

    //printf("writing %s\n", (char*)arg );  
    fout = fopen( arg, "w" );
    if ( fout == NULL ) terminate( "opening output file" );
    while( (n = queue_take(buf)) >0 ) {
        nw = fwrite( buf, 1, n, fout );
        if ( nw < n ) terminate( "write" );
    }
    fclose(fout);
    //printf("writing done\n" );    
    return NULL;
}


/**
 * main - argv (command line) must include original filename and new file
 */ 
int main( int argc, char *argv[] ) {
    pthread_t p, c;

    if ( argc != 3 )
        terminate( "wrong number of arguments [mtcopy f1 f2]" );
    if ( !queue_init( NBLK ) )
        terminate( "queue allocation error" );

    if ( pthread_create( &p, NULL, reader, argv[1])!=0 ) terminate("thread1");
    if ( pthread_create( &c, NULL, writer, argv[2])!=0 ) terminate("thread2");
    pthread_join(p, NULL);
    pthread_join(c, NULL);

    return 0;
}
