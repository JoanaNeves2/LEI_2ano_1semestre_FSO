/***
 * please give your student numbers and names, as in the example:
 * 
 *  65441 Joana Neves
 *  65815 Rui Xavier
 */

/**
 * mini-project for FSO 2023/2024 - LEI
 * DI - FCT UNL
 * Vitor Duarte
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bitmap.h"
#include "disk.h"
#include "fs.h"

#define FS_MAGIC 0xfafafefe
#define BLOCKSIZE DISK_BLOCK_SIZE // from disk.h
#define SUPERBLOCK 0              // where is superblock
#define BITMAPSTART 1             // used blocks bitmap starts at this block number

#define MAXFILENAME 63       // max file name (\0 terminated)
#define POINTERS_PER_ENT 15  // max file size = 15*block size (if only direct blocks)
#define DIRENTSIZE 128       // must be sizeof (struct fs_dirent)
#define DIRENTS_PER_BLOCK (BLOCKSIZE/DIRENTSIZE)  // for 2K blocks = 16 entries


struct fs_dirent {      // dir entry (this also works as inode)
    uint8_t isvalid;
    char name[MAXFILENAME];  // for a C string (\0 terminated)
    // inode information and blocks index:
    uint32_t size;                  // file size in bytes
    uint32_t blk[POINTERS_PER_ENT]; // index to used blocks 
                                    // (optional: last entry is for one indirect index block)
};

struct fs_superblock {
    uint32_t magic;   // magic number is here when formated
    uint32_t nblocks; // fs size in number of blocks
    uint32_t bmapsize_bytes;   // bitmap size in bytes (round up)
    uint32_t bmapsize_blocks;  // usually = 1
    uint32_t first_datablock;  // usually = 2
    uint32_t data_size;        // number of data blocks
    uint16_t blocksize;        // block size used in this filesystem
    struct fs_dirent rootdir;  // dirent describing root dir
};

union fs_block {       // uninon of all types that can be in a disk block
    struct fs_superblock super;                  // a superblock
    struct fs_dirent diri[DIRENTS_PER_BLOCK];    // directory block with dirents
    bitmap_t bits[BLOCKSIZE / sizeof(bitmap_t)]; // bitmap block
    char data[BLOCKSIZE];   // just an array of bytes (for disk_read and disk_write)
};

struct fs_superblock rootSB;   // superblock of the mounted disk
#define ROOTDIR rootSB.rootdir // shortcut to root dir dirent of the mounted disk



/** Auxiliary functions **/


/**
 * dumpSB - prints superblock contents (for debug)
 */
void dumpSB(struct fs_superblock super) {
    printf("superblock:\n");
    printf("    magic = %x\n", super.magic);
    printf("    volume size = %u\n", super.nblocks);
    printf("    total blocks = %u\n",
            super.first_datablock + super.data_size);
    printf("    bmapsize bytes = %u\n", super.bmapsize_bytes);
    printf("    bmapsize blocks = %u\n", super.bmapsize_blocks);
    printf("    first data block = %u\n", super.first_datablock);
    printf("    data size = %u\n", super.data_size);
    printf("    block size = %u\n", super.blocksize);

    printf("root dir: size=%d (%d entries), blocks: ",
            super.rootdir.size, super.rootdir.size/DIRENTSIZE);
    for (int b=0; b<POINTERS_PER_ENT; b++)
        printf("%u ", super.rootdir.blk[b]);
    putchar('\n');
}


/**
 * name2number - finds the directory entry number for 'name'
 *      returns its dirent number  or -1 if doesn't exist
 */
int name2number(char *name) {
    union fs_block block;

    // TODO

    for(int i = 0; i < (ROOTDIR.size + BLOCKSIZE - 1)/ BLOCKSIZE; i++){ // Ver o size
        disk_read(ROOTDIR.blk[i], block.data);
        for(int j = 0; j < DIRENTS_PER_BLOCK; j++){
            if(block.diri[j].isvalid){
                if(strcmp(block.diri[j].name, name) == 0)
                    return j + (i * DIRENTS_PER_BLOCK);
            }
        }
    }

    return -1; // not found
}

/**
 * alloc_block - allocates an unused data block -> sets its bit in the bitmap
 * 		returns its number  or -1 if disk full
 */
int alloc_block() {
    union fs_block block;
    int nblk = 0; // bitmap will start at block 1

    for (int i = 0; i < rootSB.nblocks; i++) {
        if (i % (BLOCKSIZE * 8) == 0) { // start of a new bitmap block
            disk_read(++nblk, block.data);
        }
        if (bitmap_read(block.bits, i % BLOCKSIZE) == 0) {
            bitmap_set(block.bits, i % BLOCKSIZE);
            disk_write(nblk, block.data); // update disk
            return i;
        }
    }

    return -1; // no disk space
}

/**
 * add_dirent - add a name to the root directory, creating its dirent
 *      (there is no delete so just appends to dir)
 *      returns the number of the direntry used or -1 if no more space in the directory
 */
int add_dirent(char *name) {
    union fs_block block;

    int blk = ROOTDIR.size / BLOCKSIZE; // index of block to append
    if (blk>=POINTERS_PER_ENT) return -1;

    int offset = ROOTDIR.size % BLOCKSIZE;
    if (offset == 0) { // needs a new block
        ROOTDIR.blk[blk] = alloc_block();
        if (ROOTDIR.blk[blk] == -1)  return -1;
        memset(block.data, 0, BLOCKSIZE);
    } else
        disk_read(ROOTDIR.blk[blk], block.data);

    int ent = offset / DIRENTSIZE;
    assert(offset % DIRENTSIZE == 0);
    block.diri[ent].isvalid = 1;
    strncpy(block.diri[ent].name, name, MAXFILENAME);
    block.diri[ent].size = 0;
    block.diri[ent].blk[0] = 0; // just in case
    disk_write(ROOTDIR.blk[blk], block.data);
    ROOTDIR.size += DIRENTSIZE;
    block.super = rootSB;
    disk_write(SUPERBLOCK, block.data);

    assert(ent == rootSB.rootdir.size / DIRENTSIZE - 1);
    return rootSB.rootdir.size / DIRENTSIZE - 1;
}


/** Filesystem API **/

/**
 * fs_format - initialize the disk filesystem (erase its contents)
 */
int fs_format() {
    union fs_block block;

    if (rootSB.magic == FS_MAGIC) {
        printf("Cannot format a mounted disk!\n");
        return 0;
    }
    if (sizeof(block) != BLOCKSIZE ||
        sizeof(struct fs_dirent) * DIRENTS_PER_BLOCK != BLOCKSIZE) {
        printf("Disk block and FS block mismatch:\n");
        printf("%lu\t", sizeof(block));
        printf("%lu\t", DIRENTS_PER_BLOCK * sizeof(struct fs_dirent));
        printf("%d\n", BLOCKSIZE);
        return 0;
    }
    memset(&block, 0, sizeof(block));

    uint32_t nblocks = disk_size();

    // TODO: build superblock

    block.super.magic = FS_MAGIC;
    block.super.nblocks = nblocks;
    block.super.bmapsize_bytes = (nblocks + 7) / 8;
    block.super.bmapsize_blocks = 1;
    block.super.first_datablock = 2;
    block.super.data_size = nblocks - block.super.first_datablock;
    block.super.blocksize = BLOCKSIZE;

    block.super.rootdir.isvalid = 1;
    strncpy(block.super.rootdir.name, "/", MAXFILENAME);
    block.super.rootdir.size = 0;
    for(int i = 0; i < POINTERS_PER_ENT; i++){
        block.super.rootdir.blk[i] = 0;
    }

    /* writes superblock */
    disk_write(SUPERBLOCK, block.data);
    dumpSB(block.super);

    // init bitmap - starts with all blocks free (clear)
    // lets assume that bitmap fits in just one block
    assert( BLOCKSIZE*8>=nblocks);
    bitmap_t *bitmap = bitmap_alloc(BLOCKSIZE*8);  
    bitmap_set(bitmap, 0);  // SB uses block 0
    bitmap_set(bitmap, 1);  // bitmap uses block 1
    //printf("write bitmap block in disk block 1\n");
    disk_write(SUPERBLOCK + 1, (char*)bitmap);

    return 1;
}


/**
 * fs_lsdir - list files in directory
 *      print all the used entries
*/
int fs_lsdir() {
    union fs_block block;
    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return 0;
    }
    printf("entry\tsize\tname\tblocks\n");
    struct fs_dirent rootdir = ROOTDIR;

    //TODO
    for(int i = 0; i < (rootdir.size + BLOCKSIZE - 1)/ BLOCKSIZE; i++){
        disk_read(rootdir.blk[i], block.data);
        
        for(int j = 0; j < DIRENTS_PER_BLOCK; j++){
            struct fs_dirent current = block.diri[j];
            if(current.isvalid){
                printf("%d\t%d\t%s\t", j, current.size, current.name);

                for(int k = 0; k < POINTERS_PER_ENT; k++){
                    printf("%d ", current.blk[k]);
                }

                printf("\n");
            }
        }
    }
    return 1;
}



/**
 * fs_debug - print filesystem details and its contents for debugging
 *
 */
void fs_debug() {
    union fs_block block;
    struct fs_superblock sb;

    disk_read(SUPERBLOCK, block.data);
    sb = block.super;

    if (sb.magic != FS_MAGIC) {
        printf("disk unformatted !\n");
        return;
    }
    dumpSB(sb);

    printf("bitmap:\n");
    disk_read(SUPERBLOCK+1, block.data);
    bitmap_print((bitmap_t *)block.data, sb.nblocks); // assuming bitmap fits in a block
}



/**
 * fs_mount - mount a formated disk; this means the global var rootSB is
 *      initialized with the disk superblock if the disk is formated
 *      it's an error is the disk isn't formated or is already mounted
 *      returns 0 if failed
 */
int fs_mount() {
    union fs_block block;

    if (rootSB.magic == FS_MAGIC) {
        printf("a disc is already mounted!\n");
        return 0;
    }

    disk_read(SUPERBLOCK, block.data);
    if (block.super.magic != FS_MAGIC) {
        printf("cannot mount an unformatted disc!\n");
        return 0;
    }
    if (block.super.nblocks != disk_size()) {
        printf("file system size and disk size differ!\n");
        return 0;
    }
    rootSB = block.super; // initialize global variable for superblock
                          // now the disk is mounted (for this exercise)
    return 1;
}

/**
 * fs_umount - unmount the disk; this means the global var rootSB is cleared 
 */
int fs_umount() {
    if (rootSB.magic != FS_MAGIC) {
        printf("no disc is mounted\n");
    }
    memset(&rootSB, 0, sizeof(struct fs_superblock) );
    return 1;
}



/**
 * fs_open - find the file with 'name'
 * 		return the number of its dirent -> entry 0 in first directory block is
 *		number 0; the entry 0 of the second block is DIRENTS_PER_BLOCK, etc...)
 *      return -1 if not found
 */
int fs_open(char *name) {

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }

    return name2number(name);
}


/**
 * fs_create - creates a new file 'name'
 *      this adds a new dirent to directory ('name' can't exist)
 *      returns the number of dirent or -1 ir error
 */
int fs_create(char *name) {
    int ent;

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    ent = name2number(name);
    if (ent != -1) {
        printf("%s already exists\n", name);
        return -1;
    }

    // create an new entry
    return add_dirent(name); // add to the directory
}

int convertOffSetToBlock(struct fs_dirent dir, int offset){
    int block = offset / BLOCKSIZE;
    if(block < POINTERS_PER_ENT)
        return dir.blk[block];
    else{
        return -1;
    }
}

void getDirentByInumber(int inumber, struct fs_dirent *dirent){
    union fs_block block;
    int numBlock;
    int dirNum;

    numBlock = (inumber / DIRENTS_PER_BLOCK) + 2;
    dirNum = inumber % DIRENTS_PER_BLOCK;

    disk_read(numBlock, block.data);

    *dirent = block.diri[dirNum];
}

/**
 * fs_read - read length data bytes starting at offset, from the file
 * 		with id (dirent number) 'inumber'
 * 		returns number of read bytes or -1 if error
 */
int fs_read(int inumber, char *data, int length, int offset) {

    struct fs_dirent dirent;
    int bytesRead = 0;
    union fs_block buff;
    int offsetCurrent;
    char *dst;
    int bytesLeft;
    int currentBlock;
    int nCopy;
    int offsetInBlock;

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }

    // TODO

    getDirentByInumber(inumber, &dirent);

    if(dirent.isvalid == 0){
        return -1;
    }

    if(offset > dirent.size){
        return -1;
    }
        
    if(dirent.size > (offset + length))
        bytesRead = length;
    else
        bytesRead = dirent.size - offset;

    offsetCurrent = offset;
    dst = data;
    bytesLeft = bytesRead;

    while(bytesLeft > 0){

        currentBlock = convertOffSetToBlock(dirent, offsetCurrent);
        
        disk_read(currentBlock, buff.data);

        offsetInBlock = offsetCurrent % BLOCKSIZE;
        
        if(bytesLeft < BLOCKSIZE - offsetInBlock)
            nCopy = bytesLeft;
        else
            nCopy = BLOCKSIZE - offsetInBlock;

        memcpy(dst, buff.data + offsetInBlock ,nCopy);

        dst += nCopy;
        bytesLeft -= nCopy;
    offsetCurrent += nCopy;
    }
    
    return bytesRead;
}

/**
 * fs_write - write length data bytes starting at offset to the file
 * 		with id (dirent number) 'inumber'
 * 		returns number of writen bytes or -1 if error
 */
int fs_write(int inumber, char *data, int length, int offset) {

    struct fs_dirent dirent;
    int bytesWriten = 0;
    union fs_block buff;
    int offsetCurrent;
    char *dst;
    int bytesLeft;
    int currentBlock;
    int nCopy;
    int offsetInBlock;

    if (rootSB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }

    // TODO

    getDirentByInumber(inumber, &dirent);

    if(dirent.isvalid == 0){
        return -1;
    }

    if(offset > dirent.size){
        return -1;
    }
        
    bytesWriten = 0;

    if((POINTERS_PER_ENT * BLOCKSIZE) >= dirent.size + length){
        offsetCurrent = offset;
        dst = data;
        bytesLeft = length;
    }else{
        return -1;
    }

    while(bytesLeft > 0){
        currentBlock = convertOffSetToBlock(dirent, offsetCurrent);
        offsetInBlock = offsetCurrent % BLOCKSIZE;

        if(currentBlock <= 0){
            currentBlock = alloc_block();
            if(currentBlock == -1){
                return -1;
            }
            dirent.blk[offsetCurrent / BLOCKSIZE] = currentBlock;
        }

        disk_read(currentBlock, buff.data);

        if(bytesLeft < BLOCKSIZE - offsetInBlock)
            nCopy = bytesLeft;
        else
            nCopy = BLOCKSIZE - offsetInBlock;

        memcpy(buff.data + offsetInBlock, dst,nCopy);

        disk_write(currentBlock, buff.data);

        dst += nCopy;
        bytesLeft -= nCopy;
        offsetCurrent += nCopy;
        bytesWriten += nCopy;
    }

    if(offsetCurrent > dirent.size){
        dirent.size = offsetCurrent;

        int dirBlock = (inumber / DIRENTS_PER_BLOCK) + 2;
        int dirNum = (inumber % DIRENTS_PER_BLOCK);

        disk_read(dirBlock, buff.data);

        buff.diri[dirNum] = dirent;

        disk_write(dirBlock, buff.data);
    }

    return bytesWriten; //a
}