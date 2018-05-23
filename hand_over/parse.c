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
  static int limit = 0;
  int index = 0;
  if (process->argv == NULL) {
    process->argv = (char **)malloc(sizeof(char *) * DEFAULT_ARGV_SIZE);
    limit = DEFAULT_ARGV_SIZE;
  } else {
    while (process->argv[index] != NULL) {
      index++;
    }
  }  
  for ( ; !is_special_character(*character) && character != '\0'; index++) {
    // Resize
    // limit - 1 to reserve the last pointer for NULL
    if (index == limit - 1) {
      process->argv = (char **)realloc(process->argv, sizeof(char *) * limit * 2);
      limit *= 2;
    }    

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
