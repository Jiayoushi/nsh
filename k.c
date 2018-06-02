#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define COMMAND_LENGTH_LIMIT  1024
#define TRUE                  1
#define PROMPT                '?'


void print_prompt() {
  printf("%c ", PROMPT);
}

void print_parse_result(struct job *job) {
  printf("%d: ", job->total_process);
  if (job->input_redirect_mode == 1) {
    printf("<");
  } else if (job->input_redirect_mode == 2) {
    printf("<<");
  }
  if (job->input_redirect_mode != 0) {
    printf("'%s' ", job->input_redirect_filename);
  }

  struct process *process = job->first_process;
  for ( ; process != NULL; process = process->next_process) {
    if (process != job->first_process) { 
      printf("| ");
    }

    int index = 0;
    // process->argv may be NULL
    for ( ; process->argv != NULL && process->argv[index] != NULL; index++) {
      printf("'%s' ", process->argv[index]);
    }
  }
  
  if (job->output_redirect_mode == 1) {
    printf(">");
  } else if (job->output_redirect_mode == 2) {
    printf(">>");
  }
  if (job->output_redirect_mode != 0) {
    printf("'%s'", job->output_redirect_filename);
  }
  printf("\n");
}

void cleanup(struct job *job) {
  struct process *process = job->first_process;
  struct process *next_process = NULL;
  for ( ; process != NULL; process = next_process) {
    next_process = process->next_process;
    free(process->argv);
    free(process);
  }
}

int main() {
  while (TRUE) {
    print_prompt();
    char buffer[COMMAND_LENGTH_LIMIT];
    // puts it here to ensure prompt is printed before read input
    if (fgets(buffer, COMMAND_LENGTH_LIMIT, stdin) == NULL) {
      break;  
    }
    struct job job = (const struct job){NULL};
    parse(buffer, &job);
    print_parse_result(&job);
    cleanup(&job);
  }

  return 0;
}
