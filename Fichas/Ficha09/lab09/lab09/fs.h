#ifndef FS_H
#define FS_H


/***  FS interface  ***/

void fs_debug();
int  fs_format();
int  fs_mount();

int  fs_open( char *fsname );

int  fs_read( int inumber, char *data, int length, int offset );

#endif
