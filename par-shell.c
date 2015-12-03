/*
	Ostap Kozak nº82535
	Francisco Maria nº81965
	Daniel Reis nº81981
*/

#include "par-shell.h"

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

int main(int argc, char * argv[]){
	/*-- init vars --*/
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
	close(f);
	 /* CREATE VERIFICATION FUNCTION WITH CODE = -1 */
	puts("Fifo Open");

	while(1) {
		// BUFF = 1024
		// char* buff[BUFF]
		read(0, buf ,strlen(buf));
		nTokens = readLineArguments(args, MAX, buf, BUFF); /* Reads arguments from terminal*/
		printf(" arg0 : %s \n", args[0]);
		printf(" arg1 : %s \n", args[1]);
		printf(" arg2 : %s \n", args[2]);

		if(!nTokens) { /* Checks for error with commands */
			fprintf(stderr,"Invalid path name \n");

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
