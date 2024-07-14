#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "fs.h"
#include "disk.h"

#define FS_MAGIC           0x00f00baa
#define DIRENTS_PER_BLOCK  32	// 4K/128=32
#define MAXFILENAME        63	// 128 bytes per dirent-> 1+63+64
#define POINTERS_PER_ENT   15	// 64/4 -4 = 15 --> max file size = 15*4K

#define FALSE 0
#define TRUE 1

#define VALID 1
#define NON_VALID 0

#define FREE 0
#define NOT_FREE 1

struct fs_superblock {
    uint32_t magic;		 // magic number is there when formated
    uint32_t nblocks;	 // fs size in number of blocks
    uint32_t ndirblocks; // number of blocks for directory
    uint32_t ndirents;	 // max number of dir entries
};

struct fs_dirent {	// dir entry (this also works as inode) -> 128 bytes
    uint8_t isvalid;
    char name[MAXFILENAME];
    uint32_t size;
    uint32_t blk[POINTERS_PER_ENT];
};

union fs_block {	// uninon of all types that can be in a disk block
    struct fs_superblock super;
    struct fs_dirent diri[DIRENTS_PER_BLOCK];
    char data[DISK_BLOCK_SIZE];
};

struct fs_superblock rootSB; // superblock of the mounted disk

unsigned char * blockBitMap; // Map of used blocks (1char=1block, not a real bitMap)

/**
 * fs_format - initialize the disk filesystem (cleans its content)
 *  
 */
int fs_format() {
    union fs_block block;

    if (rootSB.magic == FS_MAGIC) {
        printf("Cannot format a mounted disk!\n");
        return 0;
    }
    if (sizeof(block) != DISK_BLOCK_SIZE) {
        printf("Disk block and FS block mismatch:\n");
        printf("%lu\n", sizeof(block));
        printf("%lu\n", DIRENTS_PER_BLOCK * sizeof(struct fs_dirent));
        printf("%lu\n", sizeof(struct fs_superblock));
        printf("%d\n", DISK_BLOCK_SIZE);
        printf("%lu\n", sizeof(struct fs_dirent));
        return 0;
    }
    memset(&block, 0, sizeof(block));

    /*
     * NOT IMPLEMENTED for this exercise
     */
    printf("fs_format not implemented\n");


    return 1;
}

/**
 * fs_debug - print filesystem details and its contents for debugging
 * 
 */
void fs_debug() {
    union fs_block block;

    disk_read(0, block.data); // reads superblock

    if (block.super.magic != FS_MAGIC) {
        printf("disk unformatted !\n");
        return;
    }
    printf("superblock:\n");
    printf("    %d blocks\n", block.super.nblocks);
    printf("    %d dirent blocks\n", block.super.ndirblocks);
    printf("    %d dirents\n", block.super.ndirents);
    printf("**************************************\n");

    printf("inode\tsize\tname\tblocks\n");
    // TODO: list all the files found in the directory 
    int ndirblocks = block.super.ndirblocks;
    for(int i =0; i<ndirblocks; i++){
        disk_read(i+1, block.data);
        for(int j = 0; j<DIRENTS_PER_BLOCK; j++){
            if (block.diri[j].isvalid == VALID){
                printf("%d\t%d\t%s\t", i*DIRENTS_PER_BLOCK+j, block.diri[j].size, block.diri[j].name);
                for(int k=0; k<POINTERS_PER_ENT; k++){
                    printf("%d ",block.diri[j].blk[k]);
                }
                printf("\n");
            }
        }
    }

    printf("**************************************\n");

}


/**
 * dirent_load - fill ino with information from dirent number 'numde'
 */
void dirent_load(int numde, struct fs_dirent *ino) {
    union fs_block block;

    if (numde > rootSB.ndirents) {
        printf("debug: dirent number too big\n");
        ino->isvalid = 0;
        return;
    }
    // TODO: load fs_dirent data to ino
    // first directory block is disk block number 1 
    
    int blockIndex = numde / DIRENTS_PER_BLOCK;
    int blockNumber = 1+blockIndex;
    int index = numde % DIRENTS_PER_BLOCK;

    fs_read(blockNumber, block.data);

    *ino = block.diri[index];

}


/**
 * convertOffset2Block - find the disk block where the offset is located
 * 		for the file entry in 'ino'
 * 		return -1 if offset bigger than file size
 */
int convertOffset2Block(struct fs_dirent ino, int offsetCurrent) {
    int block;

    block = offsetCurrent / DISK_BLOCK_SIZE;

    if (block < POINTERS_PER_ENT) {
        //printf("-> %d\n", inode.direct[block]);
        return ino.blk[block];
    } else {
        printf("offset to big!\n");
        return -1;
    }
}


/******************************************************************/
int fs_mount() {
    union fs_block block;

    if (rootSB.magic == FS_MAGIC) {
        printf("disc already mounted!\n");
        return 0;
    }

    disk_read(0, block.data);
    if (block.super.magic != FS_MAGIC) {
        printf("cannot mount an unformatted disc!\n");
        return 0;
    }
    if (block.super.nblocks != disk_size()) {
        printf("file system size and disk size differ!\n");
        return 0;
    }

    rootSB = block.super; // initialize global variable for superblock
                          // consider disk mounted

    /*
     * more should be made when mountting a volume, but not for this exercise ...
     */

    return 1;
}

/******************************************************************/

/**
 * fs_open - find the file with 'name'
 * 		return the sequence number of its dirent/inode (entry n in
 * 		block m is (m-1)*DIRENTS_PER_BLOCK+n -> entry 0 in block 0 is
 *		number 0; the first of the second block is DIRENTS_PER_BLOCK, etc)
 * 		or -1 if not found
 */
int fs_open(char *name) {
    union fs_block block;

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    // TODO: get dirent/inode for file name in argument
    // look at each dirent in all directory blocks, looking for 'name'
    // return dirent sequence number (first dirent in first block is 0)
    for(int i =0; i<rootSB.ndirblocks; i++){
        disk_read(i+1, block.data);
        for(int j =0; j<DIRENTS_PER_BLOCK; i++){
            if (block.diri[j].isvalid == VALID && strcmp(block.diri[j].name, name)){
                return i*DIRENTS_PER_BLOCK+j;
            }
        }
    }


    return -1;
}

/**
 * fs_read - read length data bytes starting at offset from the file
 * 		with id (dirent/inode number) 'inumber'
 * 		return -1 if error
 */
int fs_read(int inumber, char *data, int length, int offset) {
    int currentBlock, offsetCurrent, offsetInBlock;
    int bytesLeft, nCopy, bytesToRead;
    char *dst;
    union fs_block buff;
    struct fs_dirent inode;

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    dirent_load(inumber, &inode);
    if (inode.isvalid == NON_VALID) {
        printf("dirent/inode is not valid\n");
        return -1;
    }
    if (offset > inode.size) {
        printf("offset bigger that file size !\n");
        return -1;
    }

    if (inode.size > (offset + length))
        bytesToRead = length;
    else
        bytesToRead = inode.size - offset; // can't read more than file size
    dst = data;
    offsetCurrent = offset;
    bytesLeft = bytesToRead;
    while (bytesLeft > 0) {	// read all blocks until fulfill bytesToRead bytes
        currentBlock = convertOffset2Block(inode, offsetCurrent);
        disk_read(currentBlock, buff.data);
        offsetInBlock = offsetCurrent % DISK_BLOCK_SIZE;

        if (bytesLeft < DISK_BLOCK_SIZE - offsetInBlock)
            nCopy = bytesLeft;
        else
            nCopy = DISK_BLOCK_SIZE - offsetInBlock;

        memcpy(dst, buff.data + offsetInBlock, nCopy);
        dst = dst + nCopy;
        bytesLeft = bytesLeft - nCopy;
        offsetCurrent = offsetCurrent + nCopy;
    }

    return bytesToRead;
}

