#include <stdio.h>
#include <pthread.h>

#include <unistd.h>

#define N	10

char buffer[N];

int ocupados=0;
int c=0, p= 0;

#define FULLBUF (ocupados==N)
#define EMPTYBUF (ocupados==0)

// see OSTEP 30.2
pthread_cond_t cndC= PTHREAD_COND_INITIALIZER;
pthread_cond_t cndP= PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx= PTHREAD_MUTEX_INITIALIZER;

void putInBuffer(char chr) {
  buffer[p]= chr;
  p= (p+1)%N;
  ocupados++;
}

void getFromBuffer(char *chr) {
  *chr= buffer[c];
  c= (c+1)%N;
  ocupados--;
}

void produz(char c) { // TODO:
    pthread_mutex_lock(&mtx);
    while(FULLBUF){
      pthread_cond_wait(&cndP, &mtx);
    }
    putInBuffer(c);
    pthread_cond_signal(&cndC);
    pthread_mutex_unlock(&mtx);

}

void consome(char *chr) { // TODO:
     pthread_mutex_lock(&mtx);
    while(EMPTYBUF){
      pthread_cond_wait(&cndC, &mtx);
    }
    getFromBuffer(chr);
    pthread_cond_signal(&cndP);
    pthread_mutex_unlock(&mtx);

}

void *produtor(void * args) {  // send the alphabet, char by char
  for (int i= 0; i < 26; i++) produz('A'+i);
  produz('\n');
  for (int i= 0; i < 26; i++) produz('a'+i);
  produz('\0');
  return NULL;
}

void *consumidor(void * args) { // receive chars and print to stdout
  char chr;
  while (1) {
      consome(&chr);
      if (chr=='\0') break;
      putchar(chr); fflush(stdout); 
      usleep(50000);  // make it slow
  }

  putchar('\n'); fflush(stdout);
  return NULL;
}


int main(int argc, char *argv[]) {
  pthread_t p1, p2;

  pthread_create(&p1, NULL, consumidor, NULL);
  pthread_create(&p2, NULL, produtor, NULL);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  return 0;
}
