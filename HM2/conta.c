//Joana Neves, 65441
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#define SIZE 1024

int current_thread = 0; //variável que diz em que thread se está a escrever a contagem atualmente
int num, nrThreads;
char *id_threads; //array de char que guarda os chars identificadores de cada thread
pthread_t *tids;
pthread_mutex_t exmut; //mutex solução para a região crítica através de uma mutual exclusion


// Code executed by each thread
void *counts_thread(void *id){
   char aux = *((char*) id);

    for(int i=0; i<=num; i++){
        pthread_mutex_lock( &exmut );
        //região crítica
            if(aux == id_threads[current_thread]){
                printf("hello from %c: %i\n", aux, i);
                current_thread = (current_thread + 1) % nrThreads;
            }
            else
                i--; //se a current_thread não seja a desta contagem, com este i-- não permite saltar números da contagem à frente
        pthread_mutex_unlock( &exmut );
    }   
    return NULL;
}

// Code with the creation of multiple threads
void counts(int nrThreads, int finalNum){
    
    //alocação de memória para os arrays necessários
    tids = (pthread_t *)malloc(SIZE * sizeof(pthread_t));
    id_threads = (char *)malloc(SIZE * sizeof(char));
    
    //criação dos threads e inserir os chars necessários para a identificação do nrThreads pedidos pelo utilizador no array id_threads
	for(int i=0; i<nrThreads;i++){
	    id_threads[i] = 'A' + i;
		pthread_create(&tids[i],NULL,counts_thread,&id_threads[i]); 
	}

    //maneira de esperar os threads uns pelos outors
	for(int i=0;i<nrThreads;i++){
		pthread_join(tids[i],NULL);
	}
}

int main( int argc, char *argv[]){

    nrThreads = atoi(argv[1]); //nº de threads que o utilizador deseja
    num = atoi(argv[2]); // nº final que o utilizador deseja que os threads terminem as suas contagens
    
    pthread_mutex_init( &exmut, NULL); //iniciaçização do mutex

    counts(nrThreads,num);
    
    pthread_mutex_destroy(&exmut); //destruição do mutex

    return 0;
}