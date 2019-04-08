CC=gcc
CFLAGS=
OBJ=main.o shell.o parse.o
exe=xsh

nsh: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o xsh

main.o: main.c parse.h shell.h
	$(CC) $(CFLAGS) -c main.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

clean: 
	rm $(OBJ) xsh
