/*
 * sish - FSO LEI/MIEI
 *
 * vad DI FCT UNL
 * 18/19 - 23/24
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


#define ARGVMAX 100
#define LINESIZE 1024


/* makeargv - build an argv vector from words in a string
 * in: s points a text string with words
 * out: argv[] will point to all words in the string s (*s is modified!)
 * pre: array provided by caller: argv should be declared as char *argv[ARGVMAX]
 * return: number of words pointed to by argv (or -1 in case of error)
 */
int makeargv(char *s, char *argv[ARGVMAX]) {
    int ntokens = 0;

    if (s == NULL || argv == NULL || ARGVMAX == 0)
        return -1;
    argv[ntokens] = strtok(s, " \t\n");
    while ((argv[ntokens] != NULL) && (ntokens < ARGVMAX)) {
        ntokens++;
        argv[ntokens] = strtok(NULL, " \t\n");
    }
    argv[ntokens] = NULL; // it must terminate with NULL
    return ntokens;
}





/* prompt - print a prompt string
 */
void prompt() {
    printf("sish> ");
    fflush(stdout); //writes the prompt
}

void waitloop(int pid){
		int p;
	do{
		p = wait(NULL);
		if(p==-1) perror("wait");
	}while(p!=pid);
}

void runcmd(char *cmd){
	int last = strlen(cmd)-1; //quando o comando é "\n" dá erro pois o last fica a -1 
	//if(last<0) printf("comando: enter\n"); //solução do erro da linha anterior
	//else {
		int bg = 0;
		if( cmd[last]=='&'){
			cmd[last]='\0';
			bg=1;
		}
		pid_t pid = fork();
		switch (pid){
			case 0:
				execlp(cmd,cmd,NULL);
				perror(cmd);
				exit(1);
			case -1:
				perror("fork");
				break;
			default:
				//se é background não espera que o programa termina apenas continua 
				//a pedir novos comandos com novos shells
				if(!bg) 
					waitloop(pid);
					//int defunct_pid = waitpid(pid,NULL,0);
			/*
				printf("Criei o %d\n", pid);
				childpid = wait (&st);
				if(WIFEXITED(st))
					printf("%d saiu com %d\n", childpid, WEXITSTATUS(st))=;
				else
					printf("%d terminou ou parou\n", childpid);
			*/
		}
	//}
}

/******  MAIN  ******/
int main() {
    char line[LINESIZE];


    while (1) {
        prompt();
        if (fgets(line, LINESIZE, stdin) == NULL) break;  // EOF
	// TODO:
	
	//2ªmaneira
	char *argv[ARGVMAX];
	if(strlen(line)-1<=0){
		printf("comando: enter\n"); //solução do erro quando o comando é "\n"
		//break;
	}
	else{
		makeargv(line,argv);
		if( strcmp(argv[0], "exit")==0) break; //termina
		else 
			runcmd(line);
	}
	
	//1ªmaneira que se fez na aula que nao abrange quando o exit tem espaços antes
		/*line[strlen(line)-1]='\0';
		if(strcmp(line,"exit")==0) break;
		else 
			runcmd(line);
		*/
	
    }
    return 0;
}

