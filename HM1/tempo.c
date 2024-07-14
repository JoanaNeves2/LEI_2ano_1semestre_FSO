#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
	//TODO: é preciso confirmar se existem argumentos suficientes ?? argc>=2
	
	float elapsed = 0;
	struct timeval t1,t2;
	int redirectFile = 0;

	char *name = argv[argc - 1];
	
	if(name[0] == '='){
		redirectFile = 1;
		//eliminar o primeiro caracter ('=')
		//TODO: ver se está bem feito e perguntar se não existia um processor melhor e mais rápido
		for(int i=1; i<strlen(name);i++){
			name[i-1]=name[i];
		}
		//eliminar o último caracter repetido no nome do ficheiro
		name[strlen(name)-1] = '\0'; 
	}

	char *cmd[argc];

	for(int i=1; i<argc;i++){
		cmd[i-1] = argv[i];
	}
	cmd[argc-1] = NULL;

	gettimeofday(&t1, NULL);

	pid_t pid = fork();
	
	if(pid ==-1){
			perror("fork");
			exit(1);
	}else if(pid == 0){
			
			//redireciona o canal de output para o ficheiro name, se o redirectFile == 1
			if(redirectFile){
				close(1); //TODO: verificar se é necessário fechar, se sim se está bem localizado
				int file = open(name, O_WRONLY | O_APPEND | O_CREAT, 0666);
				if(file == -1){
					perror(name);
					exit(1);
				}
			}
				
			
			//TODO: ver se esta parte do gettimeofday está correta e a do execlp também ou é execvp(cmd, )
			 //t1 recebe o tempo em que iniciou o comando
			execvp(cmd[0],cmd); //execução do comando pelo processo filho
			// t2 recebe o tempo em que terminou o comando
			perror(cmd[0]);
			exit(1);
	}else{
			int st;
			//espera até ao processo filho termine
			waitpid(pid,&st,0);
			
			gettimeofday(&t2, NULL);
			//if(WIFEXITED(st)){
				//faz a conta do tempo e escreve-o na consola

			elapsed = (float)(t2.tv_sec - t1.tv_sec) + (float)(t2.tv_usec - t1.tv_usec) / 1000000;
			printf("\nElapsed time = %.6f s\n", elapsed);

			//}
			/*else {
				perror("waitpid");
				exit(1);
			}*/
	}


	return 0;
}
