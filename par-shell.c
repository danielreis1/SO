/*
	Ostap Kozak nº82535
	Francisco Maria nº81965
	Daniel Reis nº81981
*/
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
#include "commandlinereader.h"
#include "list.h"

#define MAX 7 /* Maximum number os terminal command allowed */
#define MAXPAR 3 /* Maximum number of processes allowed */
#define MAXBUFFER 50
#define FILENAME "log.txt" /* Log file name */
#define PIPENAME "/tmp/par-shell-in" /* Name of the pipe */
#define BUFF 1024


/* ----- Function Declaration ----- */
void mutexLock(pthread_mutex_t *mutex);
void mutexUnlock(pthread_mutex_t *mutex);
void condWait(pthread_cond_t *cond, pthread_mutex_t *mutex);
void condSignal(pthread_cond_t *cond);
void writeEndFile(FILE *fp, int execTime);
void iterationSearch(FILE *fp);
void FlushFile(FILE* fp);

void createFile(FILE *fp, int processPID);
/* ----- End of Function Declaration ----- */

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

/* ------------------------------------------------------------ FUNCTIONS ---------------------------------------------------------*/

/* Function to run by a thread */
void * tarefaMonitora(){
	while(1){
//  tratamento de signal SIGINT

		mutexLock(&sem);
		while(childRunning == 0){
			condWait(&thread_condition_var,&sem); /* ----------------- Conditional Variable Wait ------------------ */
		}
		mutexUnlock(&sem);

		mutexLock(&sem); /* -------------- Mutex Lock ---------------- */
		if(numChilds>0){
			mutexUnlock(&sem); /* -------------- Mutex Unlock ---------------- */

			if(!(pid = wait(&childStatus))){
				fprintf(stderr, "Error while waiting for child to finish \n");
				exit(EXIT_FAILURE);
			} /* Waits for the terminations of the child process */

			mutexLock(&sem);	/* -------------- Mutex Lock ---------------- */

			iterationSearch(fp);
			processTime = update_terminated_process(list, pid, time(NULL),fp); /* Update endtime of the process */
			writeEndFile(fp,processTime); /*  Write to a file */
			numChilds--;
			childRunning--;
			condSignal(&condition_var); /* ----------------- Conditional Variable Signal ------------------ */
			mutexUnlock(&sem);	/* -------------- Mutex Unlock ---------------- */
			FlushFile(fp);

		}
		else if(numChilds == 0 && exitRequest == 1){
			mutexUnlock(&sem); /* -------------- Mutex Unlock ---------------- */
			break; /* If number of child processes is zero and 'exit' commmand were given, then it breaks out of the while loop end
			terminates the thread */
		}
		mutexUnlock(&sem); /* -------------- Mutex Unlock ---------------- */
	}
	pthread_exit(NULL);
}

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

int main(int argc, char * argv[]){

	char *args[MAX];	/* Array to store arguments from terminal */
	int nTokens;	/* Number of arguments token*/
	list = lst_new(); /* Initialize list */

	/* ------------------------------------- INITIALIZATION AREA ----------------------------------- */
	/* Initialize file */
	if((fp = fopen(FILENAME, "a+")) == NULL) {
		fprintf(stderr,"Error opening file");
		exit(EXIT_FAILURE);
	}

	/* Initialize mutex */
	if(pthread_mutex_init(&sem, NULL)) {
		fprintf(stderr,"Error creating semaphore \n");
		exit(EXIT_FAILURE);
	}

	/* Create thread */
	if(pthread_create(&monitora, NULL, tarefaMonitora, NULL)) {
		fprintf(stderr,"Error creating thread \n");
		exit(EXIT_FAILURE);
	}

	/* Named Pipe */
	unlink(PIPENAME);
	int code = mkfifo(PIPENAME, S_IRUSR | S_IWUSR);
	if(code == -1) {
		fprintf(stderr, "mkfifo returned an eror \n");
	}

	/* CREATE VERIFICATION FUNCTION WITH CODE = -1*/

	/* ------------------------------------- END OF INITIALIZATION AREA ----------------------------------- */
	int f = open(PIPENAME, O_RDONLY); /* Open NamedPipe */
	if(f == -1) {
		fprintf(stderr, "Cannot open FIFO for read \n");
		return EXIT_FAILURE;
	}

	close(STDIN_FILENO); /* Close std_out channel */
	dup(f); /* Redirects stdin to the file */

	 /* CREATE VERIFICATION FUNCTION WITH CODE = -1 */
	puts("Fifo Open");

	while(1) {
		// BUFF = 1024
		// char* buff[BUFF]


		nTokens = readLineArguments(args, MAX, buf, BUFF); /* Reads arguments from terminal*/
		printf(" arg0 : %s \n", args[0]);
		printf(" arg1 : %s \n", args[1]);
		printf(" arg2 : %s \n", args[2]);

		if(!nTokens) { /* Checks for error with commands */
			fprintf(stderr,"Invalid path name \n");
			FlushFile(stderr);

		} else if (nTokens == -1) {
				exit(EXIT_FAILURE);

		} else {
			/* Checks if 'exit' command was entered. If so, then the parent process will wait until all child processes are finished.
			Otherwise new process will be creaded based on given arguments. */
			if(!strcmp(args[0],"exit")) {

				exitRequest = 1; /* Tells the thread that 'exit' command was given */
				mutexLock(&sem);
				childRunning++;
				condSignal(&thread_condition_var); /* ----------------- Conditional Variable Signal ------------------ */
				mutexUnlock(&sem);
				pthread_join(monitora, NULL); /* Waits for the thread to finish */
				lst_print(list); /* Print the process list */
				fileClose(fp); /* Close the file */
				lst_destroy(list); /* Free the list */
				mutexDestroy(&sem); /* Destroy mutex */
				exit(-1);


			} else {

					mutexLock(&sem);
					while(childRunning>=MAXPAR){
						condWait(&condition_var,&sem);	/* ----------------- Conditional Variable Wait ------------------ */
					}
					mutexUnlock(&sem);
					pid = fork();

					if(pid >= 0){ /* Fork Succeeded */

						if(pid == 0){
							/* Child Process */
							createFile(out, getpid()); /* Creates file with PID name */
							redirect = open(finalName, O_WRONLY); /* Open file */
							close(STDOUT_FILENO); /* Close std_out channel */
							dup(redirect); /* Redirects stdout to the file */
							free(finalName);
							execv(args[0], args);
							fprintf(stderr,"Unable to execute a process \n");
							pthread_exit(NULL);
							exit(EXIT_FAILURE);
						} else {

							/* Parent Process */
							mutexLock(&sem);/* -------------- Mutex Lock ---------------- */
							childRunning++;
							insert_new_process(list, pid, time(NULL)); /* adds process to the list */
							numChilds++;
							condSignal(&thread_condition_var); /* ----------------- Conditional Variable Signal ------------------ */
							mutexUnlock(&sem); /* -------------- Mutex Unlock ---------------- */

						}
					} else {
						fprintf(stderr,"Unable to create a process \n");
						continue;
					}
			}
		}
	}
	return 0;
}
