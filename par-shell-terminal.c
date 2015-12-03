#include "par-shell.h"


int main(int argc, char * argv[]) {
	// char stringBuffer[MAX] = "fibonacci 123";
	// le o nome da NamedPipe da shell "argv 0 ou 1 ??"
	int nTokens;
	char *args[MAX];
	/* Open fifo for write */
	int fp = open(PIPENAME, STDOUT_FILENO);
	if(fp == -1) {
		fprintf(stderr, "Cannot open fifo");
		return EXIT_FAILURE;
	}

	while(1) {

		nTokens = readLineArguments(args, MAX, buf, BUFF); /* Reads arguments from terminal*/
		printf("%s\n",args[0]);

		if(!nTokens) { /* Checks for error with commands */
			fprintf(stderr,"Invalid path name \n");

		} else if (nTokens == -1) {
			// fprintf(fp, "fibonacci 11");
			exit(EXIT_FAILURE);

		} else {
			if(!strcmp(args[0],"exit")) {
				// verificar o que falta
				close(fp);
				exit(EXIT_FAILURE);


			} else if(!strcmp(args[0],"stats")) {
				// imprime no ecra onde corre o par-shell-terminal


			} else if (!strcmp(args[0],"exit-global")) {
				write(fp, "exit", strlen("exit") );
				// permite terminar par-shell e todos os terminais ordeiramente
				// apos criar log.txt e escrever no mesmo


			} else {
				int i;
				for (i = 0; i < nTokens; i++) {
					printf("maminhas\n" );
					write(fp, args[i], strlen(args[i]));
				}
				puts("success writing");
			}
	/* Write each string in turn */
		}
	}
	return 0;
}
