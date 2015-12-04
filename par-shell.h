#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "list.h"


#define MAX 7 /* Maximum number os terminal command allowed */
#define MAXPAR 2 /* Maximum number of processes allowed */
#define MAXBUFFER 50
#define FILENAME "log.txt" /* Log file name */
#define PIPENAME "/tmp/par-shell-in" /* Name of the pipe */
#define BUFF 1024
#define PIPESIGHANDLER "/tmp/SIGHANDLER"

/* ----------------------------------------------------------- GLOBAL VARIABLES --------------------------------------------------*/
int numChilds = 0;	/* Number of child processes */
int childRunning = 0;
int pid;
int childStatus;	/* Status returned from child process */

list_t *list; 		/* List to store processes information */
pthread_t monitora; /* Thread */
pthread_mutex_t sem; /* Mutex */

int exitRequest = 0; /* Used by main thread to communicate to monitor thread that exit command was given */
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER; /* Conditional Variable */
pthread_cond_t thread_condition_var = PTHREAD_COND_INITIALIZER; /* Thread conditional variable */
FILE *fp; /* File Pointer */

int iteration = -1; /* Number of iteration */
int totaltime = 0; /* Total time information from the last saved process information */
int processTime = 0; /* Performance time of the process */
char buffer[MAXBUFFER]; /* Buffer to store informations from the file */

FILE *out;
int redirect;
int redirectPipe;
char *finalName;
char pidNum[5];
char buf[BUFF];

/* ----------------------------------------------------------- END OF GLOBAL VARIABLES -----------------------------------------------*/


/* ----- Function Declaration ----- */
void mutexLock(pthread_mutex_t *mutex);
void mutexUnlock(pthread_mutex_t *mutex);
void condWait(pthread_cond_t *cond, pthread_mutex_t *mutex);
void condSignal(pthread_cond_t *cond);
void writeEndFile(FILE *fp, int execTime);
void iterationSearch(FILE *fp);
void FlushFile(FILE* fp);
void createFile(FILE *fp, int processPID);
void handlerSIGINT(int a);

/* --------------------------- ERROR CHEKING FUNCTIONS ---------------------------------- */
void mutexLock(pthread_mutex_t *mutex){
	if(pthread_mutex_lock(mutex)){
		fprintf(stderr, "Error locking mutex \n");
		exit(EXIT_FAILURE);
	}
}

void mutexUnlock(pthread_mutex_t *mutex){
	if(pthread_mutex_unlock(mutex)){
		fprintf(stderr, "Error unlocking mutex \n");
		exit(EXIT_FAILURE);
	}
}

void fileClose(FILE *fp){
	if(fclose(fp)){
		fprintf(stderr,"Error closing file \n");
		exit(EXIT_FAILURE);
	}
}

void mutexDestroy(pthread_mutex_t *mutex){
	if(!pthread_mutex_destroy(mutex)){
		fprintf(stderr,"Error closing mutex \n");
		exit(EXIT_FAILURE);
	}
}

void condWait(pthread_cond_t *cond, pthread_mutex_t *mutex){
	if(pthread_cond_wait(cond, mutex)){
		fprintf(stderr,"Error with wait conditional variable \n");
		exit(EXIT_FAILURE);
	}
}

void condSignal(pthread_cond_t *cond){
	if(pthread_cond_signal(cond)){
		fprintf(stderr,"Error with signal conditional variable \n");
		exit(EXIT_FAILURE);
	}
}

void FlushFile(FILE* fp){
	if(fflush(fp)){
		fprintf(stderr, "Error flushing file");
		exit(EXIT_FAILURE);
	}
}

/* SIGNAL HANDLING */
void handlerStats(int a) {
  int stdout;
  FILE * fp;
  stdout = dup(1); // doubles stdout
  close(1);
  unlink(PIPESIGHANDLER);
	int code = mkfifo(PIPENAME, S_IRUSR | S_IWUSR);
	if(code == -1) {
		fprintf(stderr, "mkfifo returned an eror \n");
	}
  fp = open(PIPESIGHANDLER, "w"); /* Open NamedPipe write only*/
  dup(fp);
	if(fp == NULL) {
		fprintf(stderr, "Cannot open FIFO for read \n");
		return EXIT_FAILURE;
	}
  iterationSearch(FILE *fp);
  close(1);
  dup(stdout);
}

/* --------------------------- END OF ERROR CHECKING FUNCTIONS ---------------------------------- */

/* Search for the last process information and iteration from the file */
void iterationSearch(FILE *fp) {
	rewind(fp);
	while(fgets(buffer, MAXBUFFER, fp)!= NULL) {
		sscanf(buffer, " iteracao %d ",&iteration);
		sscanf(buffer, "total execution time: %d s ", &totaltime);
	}
	fprintf(fp,"iteracao %d\n", ++iteration);
}

/* Adds end process information to the file  */
void writeEndFile(FILE *fp, int execTime) {
	int total = execTime + totaltime;
	fprintf(fp, "total execution time: %d s\n", total);
}

/* Create file for every child process */
void createFile(FILE *fp, int processPID) {
	finalName = (char*)malloc(sizeof(char)*25);
	strcat(finalName, "par-shell-out-");
	sprintf(pidNum, "%d", processPID);
	strcat(finalName, pidNum);
	strcat(finalName, ".txt");
	if((fp = fopen(finalName,"w"))==NULL){
		fprintf(stderr,"Error opening file");
		exit(EXIT_FAILURE);
	}
	fclose(fp);
}
/* ------------------------------------------------------------- END OF FUNCTIONS --------------------------------------------------*/
