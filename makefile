compile:
	gcc -g -c commandlinereader.c par-shell.c list.c par-shell-terminal.c
	gcc -pthread -lrt -o par-shell commandlinereader.o par-shell.o list.o
	gcc -o par-shell-terminal par-shell-terminal.o commandlinereader.o list.o

	

	
