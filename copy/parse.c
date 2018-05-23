/*
A parser's job is to break up the line into its ituent logical parts. That means it needs to figure out if
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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct process *get_new_process(struct job *job) {
  struct process *new_process = (struct process *)malloc(sizeof(struct process));
  *new_process = ( struct process){NULL};
  job->total_process++;
  return new_process;
}

struct process *handle_pipe(struct process* previous_process, struct job *job) {
  struct process *new_process = get_new_process(job);
  previous_process->next_process = new_process;
  return new_process;
}

char *parse_redirection( char *character, struct job *job) {
   char token = *character;
  if (token == '<') {
    job->input_redirect_mode = 1;
  } else {
    job->output_redirect_mode = 1;
  }

  if (*(++character) == token) {
    if (token == '<') {
      job->input_redirect_mode = 2;
    } else {
      job->output_redirect_mode = 2;
    }
    character++;
  }

  while(isspace(*character)) {
    character++;
  }
  
  char *redirect_filename = (token == '<') ? job->input_redirect_filename :
    job->output_redirect_filename;

  int index = 0;
  while (isalnum(*character)) {
    redirect_filename[index] = *character;
    index++;
    character++;
  }
  redirect_filename[index] = '\0';

  return character;
}

 char special_characters[] = "<>|";

int is_special_character(int character) {
  return strchr(special_characters, character) != NULL;
}


char *parse_argv( char *character, struct process *process) {
  process->argv = (char **)malloc(sizeof(char *) * DEFAULT_ARGV_SIZE);
  
  int index = 0;
  for ( ; !is_special_character(*character) && character != '\0'; index++) {
    process->argv[index] = (char *)character; 

    while (!isspace(*character) && !is_special_character(*character) && *character != '\0') {
      character++;
    }

    while (isspace(*character)) {
      *character++ = '\0';
    }  
  }

  process->argv[index] = NULL;

  return character - 1;
}

int parse( char *character, struct job *job) {
  job->first_process = get_new_process(job);
  struct process *process = job->first_process;
  while (*character != '\0') {
    switch (*character) {
      case '>': {
        character = parse_redirection(character, job); 
        break;
      }
      case '<': {
        character = parse_redirection(character, job);
        break;
      }
      case '|': {
        process = handle_pipe(process, job);
        break;
      }
      default:  {
        if (!isspace(*character)) { 
          character = parse_argv(character, process); 
        } 
        break;
      }
    }
    character++;
  }


  return 0;
}
