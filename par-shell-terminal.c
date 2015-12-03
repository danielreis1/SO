
#include "par-shell.h" // fazer par-shell.h
#include "commandlinereader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX 7
#define PIPENAME "/tmp/par-shell-in"

int main(int argc, char * argv[]) {
	//char stringBuffer[MAX] = "fibonacci 123";


	/* Open fifo for write */
	FILE* fp = fopen(PIPENAME, "w");
	if(fp == NULL) {
		fprintf(stderr, "Cannot open fifo");
		return EXIT_FAILURE;
	}
	while(1) {
		nTokens = readLineArguments(args, MAX, buf, BUFF); /* Reads arguments from terminal*/
		if(!nTokens) { /* Checks for error with commands */
			fprintf(stderr,"Invalid path name \n");

		} else if (nTokens == -1) {
			fprintf(fp, "fibonacci 11");
			exit(EXIT_FAILURE);
		} else {
			if(!strcmp(args[0],"exit")) {
				// verificar o que falta
				FlushFile(fp);
				fclose(fp);
				exit(EXIT_FAILURE);
			else if(!strcmp(args[0],"stats")) {
				// imprime no ecra onde corre o par-shell-terminal

			} else if (!strcmp(args[0],"exit-global")) {
				// permite terminar par-shell e todos os terminais

			} else {
				int i;
				for (i= 0; i < len(args); i++)
					fprintf(fp,args[i]);
				puts("success writing");
				FlushFile(fp);
			}
	/* Write each string in turn */
	}
	return 0;
}
