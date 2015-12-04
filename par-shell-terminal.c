#include "par-shell.h"

int main(int argc, char * argv[]) {
	char bufer[MAXBUFFER];
	// char stringBuffer[MAX] = "fibonacci 123";
	// le o nome da NamedPipe da shell "argv 0 ou 1 ??"
	// char *args[MAX];
	/* Open fifo for write */
	printf("%s\n", argv[1]);
	int fp = open(argv[1],  O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if(fp == -1) {
		fprintf(stderr, "Cannot open fifo");
		return EXIT_FAILURE;
	}
	while(1) {
		if(fgets(bufer, MAXBUFFER, stdin) == NULL)
			printf("error in fgets\n" );

		printf("%s\n",bufer);
		if(!strcmp(bufer,"exit\n")) {
			fsync(fp);
			puts("success writing");
			// verificar o que falta
			close(fp);
			exit(EXIT_FAILURE);

		} else if(!strcmp(bufer, "stats\n")) {
			FILE* log;
			if((log = fopen(FILENAME,"r"))==NULL){
				fprintf(stderr,"Error opening file");
				exit(EXIT_FAILURE);
			}
			iterationSearch(log);
			// prints tempo total em execucao
			printf("%d\n", totaltime);
			fileClose(log);

			// imprime no ecra onde corre o par-shell-terminal


		} else if (!strcmp(bufer, "exit-global\n")) {
			char* exitC = "exit\n";
			fsync(fp);
			write(fp, exitC, strlen(exitC));
			puts("success writing");
			// apos criar log.txt e escrever no mesmo
			// permite terminar par-shell e todos os terminais ordeiramente

		} else {
			fsync(fp);
			write(fp, bufer, strlen(bufer));
			puts("success writing");
		}
/* Write each string in turn */
	}
	return 0;
}
