/*
A parser's job is to break up the line into its constituent logical parts. That means it needs to figure out if
there is any 

1. input/output redirection (the < and > characters), 
2. how many pipes there are on the command line (which is one less than the number of commands between pipes---eg., “who | wc” is two commands separated by one pipe), and
3. what the command line arguments are to each command. 

1. You should create a struct which will hold a full command
line with all this information, and put its definition into a file called parse.h. 
2. The file parse.h also contains a prototype
for the function Parse, which will be defined in the file parse.c. 

Then create parse.c, which contains the code for the
actual parser. 

The file main.c will contain a loop which (for now) just reads a line, pasess that line to the function
Parse. Parse will populate the struct with the logical info on the command line, and then your main program will
print out the parsed version of the command line. For example, given “cat –v <infile | grep foo | wc > outfile”, the Part
1 version of your main program should print:
 3: <’infile’ ‘cat’ ‘-v’ | ‘grep’ ‘foo’ | ‘wc’ > ‘outfile’  
 The ‘3’ represents the number of commands, and each “word” is printed with quotes around it. Input/output redirection
 should only be printed if they are present. So for example “who | wc” should give:
  2: ‘who’ | ‘wc’
  A correct executable of the parser is in ~wayne/pub/cs146/nsh-parser. You can use it to see what yours should output
  for Part 1 of the assignment. */

#include "parse.h"

#include <stdio.h>

int parse(const char *command, job job) {
  const char *iterator;

  for (iterator = command; *iterator != '\0'; iterator++) {
    const char character = *iterator;

    switch (character) {
      
    }
    
  }


  return 0;
}
