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

struct process *parse_pipe(struct process* previous_process, struct job *job) {
  struct process *new_process = get_new_process(job);
  previous_process->next_process = new_process;
  return new_process;
}

char special_characters[] = "<>|&";

int is_special_character(int character) {
  return strchr(special_characters, character) != NULL;
}

int is_common_character(int character) {
  return !is_special_character(character) && character != '\0' && !isspace(character);
}

char *parse_redirection( char *character, const char token, struct job *job) {
  if (token == '<') {
    job->input_redirect_mode = 1;
  } else {
    job->output_redirect_mode = 1;
  }

  if (*character == token) {
    job->output_redirect_mode = 2;
    character++;
  }

  while(isspace(*character)) {
    character++;
  }
 
  
  char *redirect_filename = (token == '<') ? job->input_redirect_filename :
    job->output_redirect_filename;

  int index = 0;
  while (is_common_character(*character)) {
    redirect_filename[index++] = *character;
    character++;
  }
  redirect_filename[index] = '\0';

  return character - 1;
}



char *parse_argv(char *character, struct process *process) {
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
		while (is_common_character(*character)) {
			if (*character == '\\') {
				char *c = character;
				while (*c != '\0') {
					*c = *(c + 1);
					c++;
				}
				character++;
			} else if (*character == '\'') {
				// 'abc def g 'xxx'zzb cc'\0
				char *c = character;
        while (*c != '\0') {
          *c = *(c + 1);
          c++;
        }
        // abc def g 'xxx'zzb cc'\0
        c = strchr(character, '\'');
        character = c;
        while (*c != '\0') {
          *c = *(c + 1);
          c++;
        }
        // abc def g xxx'zzb cc'\0
			} else {
				character++;
			}
		}
    

    // Set boundary
    // In the case of who|wc, the boundary is set in the parse function
    while (isspace(*character)) {
      *character++ = '\0';
    }  
  }

  process->argv[index] = NULL;

  return character - 1;
}

void parse_background_job(struct job *job) {
  job->background = TRUE;
}

// All sub-parse functions should return the character
// it has parsed
int parse(char *command, struct job *job) {
  char *character = command;  
  char *pound = strchr(character, '#');
  if (pound != NULL) {
    *pound = '\0';
  }  

  if (strlen(character) <= 1) {
    return 0;
  }
  job->first_process = get_new_process(job);
  struct process *process = job->first_process;
  while (*character != '\0') {
    switch (*character) {
      case '>': {
        *character = '\0';
        character = parse_redirection(character + 1, '>', job); 
        break;
      }
      case '<': {
        *character = '\0';
        character = parse_redirection(character + 1, '<', job);
        break;
      }
      case '|': {
        *character = '\0';
        process = parse_pipe(process, job);
        break;
      }
      case '&': {
        *character = '\0';
        parse_background_job(job);
        break;
      }
      default:  {
        if (is_common_character(*character)) { 
          character = parse_argv(character, process); 
        } else {
          *character = '\0';
          //printf("Uncaught [%c]", *character );
        }
        break;
      }
    }
    character++;
  }
 
  /*
  character = command;
  for ( ; *character != '\0'; character++) {
    if (!is_common_character(*character)) {
      *character = '\0';
    }
  }*/
  
  return 0;
}
