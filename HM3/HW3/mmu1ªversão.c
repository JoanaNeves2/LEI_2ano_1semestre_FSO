
/**
 *   mmu.c - simulator of a MMU and the OS memory management for on-demand paging 
 *          with page replacement and swap space.
 *  FSO - LEI
 *
 *  2023 Vitor Duarte
 *  DI - FCT UNL
 */

#include <stdio.h>
#include <stdlib.h>

/*
Using:
 - CPU with 32 bits for virtual addresses and real addresses 
 - pages with 8KB (2^13) 
 - so maximum page table size is 2^32/2^13=2^19 (512K) entries
*/
#define PAGESIZE (8*1024)    // 8KBytes
#define OFFSET_BITS 13       // 2^13 = 8K
#define MAX_VIRTUAL_PAGES (512*1024)


/*
 * real memory
 */
#define MAX_REAL_PAGES (256)  // eg. 256 * 8K = 2 MBytes


/*
 * page table
 */
unsigned int pageTable[MAX_VIRTUAL_PAGES];  // not all entries need to be in use

// page "bits" to indicate that a page isn't mapped to physical memory
// or that a page isn't valit to this process
#define NOT_MAPPED  (-1)
#define NOT_VALID   (-2)


/*
 * frames table
 */
unsigned int frameTable[MAX_REAL_PAGES];  // max real memory
int numFrames;                            // real memory to simulate
int access[MAX_REAL_PAGES]; //new frame table to simulate this bit
unsigned int frame_index;

// physical memory frame is free
#define FREE  (-1)




/*
 *  statistics:
 */
int count;      // memory access counter
int pagefaults; // page faults counter
int swapouts;   // pages swapped out counter



/**
 * initTabs - OS work:
 *          init page table of a process
 *          init frame table representing the machine's physical memory
 */
void initTabs() {
    frame_index = 0;
    for ( int i=0; i<MAX_REAL_PAGES; i++ ){ 
        frameTable[i] = FREE; // not in use
        access[i] = 0; //inicializar o vetor dos bits de acesso
    }

    for ( int i=0; i<MAX_VIRTUAL_PAGES; i++)
        pageTable[i] = NOT_MAPPED;  // not mapped
    pageTable[0] = NOT_VALID;       // not valid addresses (eg. address of NULL is here)
    pageTable[MAX_VIRTUAL_PAGES-1] = NOT_VALID; // not valid addresses
}
    


/**
 * os_pagein - load page contents to memory frame
 *          can be all zeros (if new heap page), can load from file,
 *          or can load from swap space the saved page
 */
void os_pagein(int page, int frame) {
    // just a simulation, so just print a message
    printf("swapin page %x to frame %x\n", page, frame);
}


/**
 * os_pageout - discard or save frame contents to secondary memory (swap space)
 *          to save page content
 */
void os_pageout(int page, int frame) {
    // just a simulation, so just print a message
    printf("swapout page %x from frame %x\n", page, frame);
    swapouts++;  // one more page out
}


/**
 * os_handle_invalidpage - handler to the invalid page reference exception (interrupt)
 *          usualy the OS will kill the processes
 */
void os_handle_invalidpage( unsigned page, unsigned addr ) {
    // OS  kills this process (maybe sending SIGVEC)
    // in this simulator just prints a message
    printf("ERROR: invalid address: %x (page %x)\n", addr, page);
}



/**
 * os_handle_pagefault - handler to the page fault reference exception (interrupt)
 *          usualy the OS will try to map a new frame to this page
 *          and load its contents (all zeros, from a file or from swap space) 
 */
void os_handle_pagefault( unsigned page ) {
    pagefaults++;    // one more pagefault

    // TODO: FIFO with second chance

    //Em caso de ainda haver frames FREE, livres, as páginas são alocadas ao primeiro frame livre
    for(int i = 0; i<numFrames; i++){
         if(frameTable[i] == FREE){
            frameTable[i] = page; //associa a página ao frame
            pageTable[page] = i; //associa o frame à página
            os_pagein(page, i); //output de associação da página ao frame i
            access[i] = 0; //colocar o bit de acesso do frame a zero, quer dizer que não foi acedido ainda, neste caso apenas foi associado
            return;
         }
    }

    //Em caso de já não haver frames livres   
    while(1){ //política sequencial
        //verifica se o frame foi associado recentemente ou não (access[frame_index] == 1) significa que foi acedido à pouco tempo
        if(access[frame_index] == 1){
            //altera o access para 0, não foi recentemente acedido
            access[frame_index] = 0;
            //incremente o index do último frame a ser visto
            frame_index = (frame_index+1) % numFrames;
        }
        //se for inválida e estiver mapeada tem que se utilizar a politica da substituição
        else{
            //quando o frame não é acedido recentemente então o código procedirá à política de substituição
            //output da página que está a ser substituida
            os_pageout(frameTable[frame_index], frame_index);
            //acesso colocado a 0 pois não foi acedido apenas foi associado/substituído
            access[frame_index] = 0;

            int frame = frameTable[frame_index];
            //coloca a página antiga do frame NOT_MAPPED pois está a ser substituída
            pageTable[frame] = NOT_MAPPED;
            
            //substituição para a página recente, page, com atualização na frameTable e na pageTable
            frameTable[frame_index] = page;
            pageTable[page] = frame_index; 
              
            //output da associação da nova página ao frame 
            os_pagein(page, frame_index);
            //incremente o index do último frame a ser visto
            frame_index = (frame_index+1) % numFrames;
            return;
        }
    }
}


/**
 * translateOneAddr - translate a virtual address to real address using pageTable
 *          simulate hardware MMU working
 *  returns physical address for the virtual addr
 */
unsigned int translateOneAddr(unsigned int addr) {
    int page;
    int frame;
    int realAddress=0;

    page = addr >> OFFSET_BITS;
    //printf("page:%x\n", page); // debug
    frame = pageTable[ page ];
    if ( frame == NOT_VALID ) {    // invalid page
        os_handle_invalidpage( page, addr );  // interruption to OS to handle
        return -1;  // same as 0xffffffffffff (max addr)
    }
    if ( frame == NOT_MAPPED ) {    // not mapped
        os_handle_pagefault( page );  // interruption to OS to handle
        frame = pageTable[ page ];
    }
    else{
        //modificação do bit de acesso da página em questão para 1 pois foi recentemente acedida
        access[frame] = 1;
    }
    realAddress =  (frame << OFFSET_BITS) |
           (addr & ((1 << OFFSET_BITS) - 1)); /* real address */
           
    return realAddress;
}

/**
 * simulateAllSteps - simulate all memory acesses with translation from
 *          virtual address to real addresses
 *          tf is the name of the text file with addresses, one per line
 */
void simulateAllSteps(FILE *tf) {
    unsigned Vaddr, Raddr;
    char rw;

    while (fscanf(tf, "%x %c", &Vaddr, &rw) == 2) {
        Raddr = translateOneAddr(Vaddr); // translate vitual addr
        if ( Raddr == -1 ) {
            printf("Simulated process Segmentation fault!\n");    // abort program
            return;
        }
        count++;    // one more address accessed
    }
}


/************* MAIN **************/

int main(int argc, char *argv[]) {
    FILE *traceFile;
    char *prog = argv[0];

    if (argc != 3) {
        printf("usage: %s numb_frames memtrace\n", prog);
        return 1;
    }
    numFrames = strtol(argv[1], NULL, 10);
    if ( numFrames<=0 || numFrames >MAX_REAL_PAGES) {
        printf("usage: numb_frames must be in 1..%i\n", MAX_REAL_PAGES);
        return 1;
    }
    if ((traceFile = fopen(argv[2], "r")) == NULL) {
        printf("Trace-file '%s' error\n", argv[2]);
        return 1;
    }

    /* do it */
    initTabs();
    simulateAllSteps(traceFile);
    
    /* statistics */
    printf("Memory accesses: %d\n", count );
    printf("Page faults: %d (%.2f%% of memory accesses)\n",
            pagefaults, (float)pagefaults/count*100.0);
    printf("Swap outs: %d\n", swapouts );
    return 0;
}
