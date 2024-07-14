#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define NTRIES  1000

int main(int argc, char *argv[]) {
	//TODO: é preciso confirmar se existem argumentos suficientes ?? argc>=2
	
	long elapsed;
    struct timeval t1,t2;
	int redirectFile = 0;
	char *name = argv[strlen(argv)-1];
	if(name[0] == '='){
		redirectFile = 1;
		//eliminar o primeiro caracter ('=')
		//TODO: ver se está bem feito e perguntar se não existia um processor melhor e mais rápido
		for(int i=1; i<strlen(name)-1;i++){
			name[i-1]=name[i];
		}
	}

	pid_t pid = fork();
	switch(pid){
		case 0:
			//redireciona o canal de output para o ficheiro name, se o redirectFile == 1
			if(redirectfile){
				close(1); //TODO: verificar se é necessário fechar, se sim se está bem localizado
				int file = creat(name,O_RDONLY);
				if(file == -1){
					perror("creat")
					exit(1);
				}
				
			}
			char *cmd = argv[1];
			//TODO: ver se esta parte do gettimeofday está correta e a do execlp também ou é execvp(cmd, )
			gettimeofday(&t1, NULL); //t1 recebe o tempo em que iniciou o comando
			execlp(cmd,cmd,NULL); //execução do comando pelo processo filho
			gettimeofday(&t2, NULL); // t2 recebe o tempo em que terminou o comando
			perror(cmd);
			exit(1);
		case -1:
			perror("fork");
			exit(1);
		default:
			int st;
			//espera até ao processo filho termine
			waitpid(pid,&st,0);
			if(WIFEXITED(st)){
				//faz a conta do tempo e escreve-o na consola
				elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
				printf("Elapsed time = %6li us (%g us/call)\n", elapsed, (double)elapsed/NTRIES);
			}
			else {
				perror("waitpid");
				exit(1);
			}
	}


	return 0
}