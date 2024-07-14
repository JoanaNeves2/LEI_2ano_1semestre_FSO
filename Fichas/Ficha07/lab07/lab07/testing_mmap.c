#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

void fatal_system_error(char *str){
  perror(str);
  exit(1);
}

/* 
 * mmapcopy - uses mmap to copy file fd to stdout 
 */
void mmapcopy(int fd, int size) 
{
    char *bufp; /* ptr to memory mapped VM area */
    int n;

    bufp = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if( bufp == MAP_FAILED)
      fatal_system_error("mmap ");
    n = write(1, bufp, size);
    if( n != size)
      fatal_system_error("write ");
    return;
}


int main(int argc, char **argv) 
{
    struct stat stat;
    int fd;

    /* check for required command line argument */
    if (argc != 2) {
	printf("usage: %s <filename>\n", argv[0]);
	exit(0);
    }

    /* copy the input argument to stdout */
    fd = open(argv[1], O_RDONLY, 0);
    if( fd < 0)
      fatal_system_error("error in open");
    if( fstat(fd, &stat) != 0)
      fatal_system_error("error in fstat");
	
    mmapcopy(fd, stat.st_size);
    return 0;
}

