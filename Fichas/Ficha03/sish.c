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


void executePipe(char *argv[], int bg, char *argv2[]){
	
	int i = 0;
	while(argv[i] != NULL){
		printf("%s\n", argv[i]);
		i++;
	}
	
	i = 0;
	while(argv2[i] != NULL){
		printf("%s\n", argv2[i]);
		i++;
	}


	int p [2];
	if(pipe(p) <0){
		perror("Erro no pipe");
		return;
	}

	int pid = fork();

	switch(pid){
	case -1:
		perror("Erro no fork");
     	exit(1);
	
	case 0:
		dup2(p[1], 1);
		close(p[0]);
		close(p[1]);
     	execvp(argv[0], argv);
		perror("Erro no exec");
		exit(1);

	default:
		switch(pid = fork()){
			case -1:
				perror("Erro no segundo fork");
     			exit(1);
     		case 0:
     			dup2(p[0], 0);
				close(p[1]);
				close(p[0]);
			  	execvp(argv2[0], argv2);
			  	perror("Erro no segundo exec");
			  	exit(1);
			default:
				close(p[1]);
				close(p[0]);
				if(!bg){
			 		int process;
					int stat;
			 		while ((process = wait(&stat)) != pid && process > 0){
						 printf("Process %d terminated with status %d\n", process, WEXITSTATUS(stat));
					}
					if(process == -1){
						 perror("wait");
					}
				}
		}
	}
}



void executeSimple(char *argv[], int bg){

	int pid = fork();

	switch(pid){
	case -1:
		perror("Erro no fork");
     	exit(1);
	
	case 0:
     	execvp(argv[0], argv);
		perror("Erro no exec");
		exit(1);

	default:
		if(!bg){
	 		int p;
	   	int stat;
	 		while ((p = wait(&stat)) != pid && p > 0){
			    printf("Process %d terminated with status %d\n", p, WEXITSTATUS(stat));
			}
			if(p == -1){
			    perror("wait");
			}
		}
	}
}


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
    argv[ntokens] = strtok(s, " \t\n"); // Divide a cadeia em palavras consoante as que estao no argumento
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

/******  MAIN  ******/
int main() {
    char line[LINESIZE];
	 char* argv[ARGVMAX];

    while (1) {
        prompt();
        if (fgets(line, LINESIZE, stdin) == NULL) break;  // EOF
        
        line[strlen(line) - 1] = '\0';
        
        if (strcmp(line, "exit") == 0) {
				break;        
        }
        
        int nTokens = makeargv(line, argv);
        
        int background = 0;
        
        
		  if (strcmp(argv[nTokens - 1], "&") == 0){
        		background = 1;
        		argv[nTokens - 1] = NULL;
        }
         
        int pp = 0;
        int i = 0 ;
		  char **argv2;

        
        while(argv[i] != NULL){
        		if(strcmp(argv[i], "|")== 0){
        			argv[i] = NULL;
        			argv2 = &argv[i+1];
        			pp = 1;
        		}
        		i++;
        }

		  
		  if(pp ==0){
		  		executeSimple(argv, background);
		  }
		  else{
		  		executePipe(argv, background, argv2);
		  }
		  
    }
    return 0;
}