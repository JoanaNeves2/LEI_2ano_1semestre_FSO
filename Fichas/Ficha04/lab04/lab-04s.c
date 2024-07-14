#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define SIZE 256*1024*1024
int *array;
int length, count;

void count3s(){
    count = 0;

    for (int i=0; i < length; i++){
        if (array[i] == 3){
            count++;
        }
    }
}

int main( int argc, char *argv[]){

    array= (int *)malloc(SIZE*sizeof(int));
    length = SIZE;

    srand(0);  // always the same sequence
    for (int i=0; i < length; i++){
        array[i] = rand() % 4;
    }
    count3s();
    printf("Count of 3s = %d\n", count);
    return 0;
}
