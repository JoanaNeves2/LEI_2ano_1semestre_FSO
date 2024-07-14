
# use sem_open or sem_init (on MACOS only sem_open is available) 
CFLAGS=-Wall -g -DNAMEDSEM
#CFLAGS=-Wall -g -DUNNAMEDSEM

# using sem
mtcopy:	mtcopy.c blockqueue.c
	cc $(CFLAGS) -o $@ $^ -pthread

# using 2nd implementation of blockqueue
mtcopy2: mtcopy.c blockqueue2.c
	cc $(CFLAGS) -o $@ $^ -pthread

clean:
	rm -f mtcopy mtcopy2
