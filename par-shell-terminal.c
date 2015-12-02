
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
	int fd = open(PIPENAME, O_WRONLY);
	if(fd == -1) {
		fprintf(stderr, "Cannot open fifo");
		return EXIT_FAILURE;
	}
	write(fd, "fibonacci 123", strlen("fibonacci 123"));
	puts("success writing");

	/* Write each string in turn */
	close(fd);
	return 0;
}
