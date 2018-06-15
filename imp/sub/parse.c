#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct process *allocate_process(struct job *job) {
  struct process *new_process = 
        (struct process *)malloc(sizeof(struct process));
  *new_process = (struct process){NULL};
  job->total_process++;
  return new_process;
}

char special_characters[] = "<>|&#";
int is_special_character(int character) {
  return strchr(special_characters, character) != NULL;
}

int is_common_character(int character) {
  return !is_special_character(character) && character != '\0' && !isspace(character);
}

void ungetc_wrapper(int character) {
  if (ungetc(character, stdin) == EOF) {
    perror("ungetc");
    exit(EXIT_FAILURE);
  }
}

void unget_string(char *string, int length) {
  int index = length - 1;
  for ( ;index >= 0; index--) {
    printf("unget %c;\n", string[index]);
    ungetc_wrapper(string[index]);
  }
}

void read_off_space() {
  char character = '\0';
  while (isspace((character = getchar()))) {
  
  }
  ungetc_wrapper(character);
}

// Get the next word in the command line
// Return the word if sucessful, else return null
char *get_word() {
  read_off_space();

  static char word[MAX_ARGUMENT_LENGTH];
  int index = 0;
  while (TRUE) {
    char character = getchar();
    switch (character) {
      case '\\': {
        word[index++] = getchar();
        break;
      }
      case '\'': {
        while ((character = getchar()) != '\'') {
          word[index++] = character;
        }
        break;
      }
      default:  {
        if (is_common_character(character)) {
          word[index++] = character;
        } else {
          ungetc_wrapper(character);
          word[index] = '\0';
          return index == 0 ? NULL : word;
        }
      }
    }
  }
}

// parsed_word is optional, it will be used if it's supplied.
// otherwise, get the next word
void parse_argv(struct process *process, char *parsed_word) {
  int index = 0;
  while (process->argv[index] != NULL) {
    index++;
  }
  const char *word = parsed_word == NULL ? get_word() : parsed_word;
  process->argv[index] = (char *)malloc(sizeof(char) * strlen(word) + 1);
  strcpy(process->argv[index], word);
  
  process->argv[index + 1] = NULL;
}

void parse_redirection(const char token, struct job *job) {
  if (token == '<') {
    job->input_redirect_mode = 1;
  } else {
    job->output_redirect_mode = 1;
  }

  char character = getchar();
  if (character == token) {
    job->output_redirect_mode = 2;
  } else {
    ungetc_wrapper(character);
  } 

  char *redirect_filename = (token == '<') ? job->input_redirect_filename :
    job->output_redirect_filename;
  const char *filename = get_word();
  strcpy(redirect_filename, filename);
}

struct process *parse_pipe(struct process* previous_process, struct job *job) {
  if (previous_process == NULL) {
    fprintf(stderr, "Unexpected pipe: there is no process before the pipe\n");
    exit(EXIT_FAILURE);
  }
  struct process *new_process = allocate_process(job);
  previous_process->next_process = new_process;
  return new_process;
}

void parse_background_job(struct job *job) {
  job->background = TRUE;
}

// Substitute $? with the exit status of previous_job
char *substitution(struct job *previous_job) {
  static char value[256];
  char character = getchar();
  switch (character) {
    case '?': {
      int exit_status = 0;
      if (previous_job != NULL) {
        exit_status = previous_job->exit_status;
      }
      int length = 0;
      if ((length = sprintf(value, "%d", exit_status)) < 0) {
        perror("sprintf");
        exit(EXIT_FAILURE);
      }
      return value;
    }
    default: {
      ungetc_wrapper(character);
      ungetc_wrapper('$');
      return NULL;
    }
  }
}

// Return value: 0 for sucess, 1 for empty input, 2 for EOF
int parse(struct job *job, struct job *previous_job) {
  struct process *process = NULL;
  char character = '\0';
  while ((character = getchar()) != '\n') {
    switch (character) {
      case '<': {
        parse_redirection('<', job);
        break;
      }
      case '>': {
        parse_redirection('>', job);
        break;
      }
      case '|': {
        process = parse_pipe(process, job);
        break;
      }
      case '&': {
        parse_background_job(job);
        break;
      } 
      case '#': {
        while (getchar() != '\n') {
          ;
        }
        ungetc_wrapper('\n');
        break;
      }
      case '$': {
        parse_argv(process, substitution(previous_job));
        break;
      }
      default: {
        if (character == EOF) {
          return 1;
        }
        if (isspace(character)) {
          break;
        }
        ungetc_wrapper(character);      

        if (process == NULL) {
          job->first_process = allocate_process(job);
          process = job->first_process;
        }

        parse_argv(process, NULL); 
        break;
      }
    }
  }
 
  return 0; 
}

