-#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

//metodo que executa o comando pedido através do processo filho, redirecionando o canal do output 
//para o ficheiro pedido caso seja o caso (se o último argumento de argv entregue na main for 
//iníciado com o caracter '='
void runcmd(int argc, char *cmd[], char *name, int redirectFile){
	//redireciona o canal de output para o ficheiro name, se o redirectFile == 1
	if(redirectFile){
		close(1); 
		cmd[argc-2] = NULL; //elimina o argumento "=<nome_do_ficheiro>"
		
		int file = creat(name, 0666);
		if(file == -1){
			perror(name);
			exit(1);
		}
	}
		
	execvp(cmd[0],cmd); //execução do comando pelo processo filho
	perror(cmd[0]);
	exit(1);
}

//o que o processo pai realiza: aguarda que o processo filho 
//termine e escreve na consola o output pedido
void runFather(pid_t pid, struct timeval t1){
	float elapsed = 0;
	struct timeval t2;
	int st;
	//espera até ao processo filho termine
	waitpid(pid,&st,0);
			
	gettimeofday(&t2, NULL); // t2 recebe o tempo em que terminou o comando
	if(!WIFEXITED(st)){
		perror("waitpid");
		exit(1);
	}
	//faz a conta do tempo e escreve-o na consola
	elapsed = (float)(t2.tv_sec - t1.tv_sec) + (float)(t2.tv_usec - t1.tv_usec) / 1000000;
	printf("\nElapsed time = %.6f s\n", elapsed);
}

int main(int argc, char *argv[]) {
	
	struct timeval t1;
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

	char *cmd[argc-1];

	for(int i=1; i<argc;i++){
		cmd[i-1] = argv[i];
	}
	cmd[argc-1] = NULL;
	

	gettimeofday(&t1, NULL); //t1 recebe o tempo em que iniciou o comando

	pid_t pid = fork();
	
	if(pid ==-1){
			perror("fork");
			exit(1);
	}else if(pid == 0){
			runcmd(argc, cmd, name, redirectFile);
	}else{
			runFather(pid, t1);
 
	}


	return 0;
}
