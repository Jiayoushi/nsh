CC=gcc
CFLAGS=

nsh: main.o parse.o
	$(CC) $(CFLAGS) main.o parse.o -o nsh

main.o: main.c parse.h
	$(CC) $(CFLAGS) -c main.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

clean: 
	rm *.o nsh
