#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

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

  printf("bg:%d", job->background);
  printf("\n");
}

void cleanup_job(struct job *job) {
  struct process *process = job->first_process;
  struct process *next_process = NULL;
  for ( ; process != NULL; process = next_process) {
    next_process = process->next_process;
    free(process->argv);
    free(process);
  }
}

void execute_job(struct job *job) {
  if (job->total_process == 0) {
    return;
  }

  int input_file_descriptor = STDIN_FILENO;
  // Setup input redirection
  if (job->input_redirect_mode == 1) {
    if ((input_file_descriptor = open(job->input_redirect_filename, O_RDONLY, NULL)) == -1) {
      perror(job->input_redirect_filename);
      exit(EXIT_FAILURE);
    }
  }
  
  // Setup output redirection
  int output_file_descriptor = STDOUT_FILENO;
  if (job->output_redirect_mode == 1) {
    if ((output_file_descriptor = open(job->output_redirect_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
      perror(job->output_redirect_filename);
      exit(EXIT_FAILURE);
    }
  } else if (job->output_redirect_mode == 2) {
    if ((output_file_descriptor = open(job->output_redirect_filename, O_WRONLY | O_APPEND)) == -1) {
      perror(job->output_redirect_filename);
      exit(EXIT_FAILURE);
    } 
  }

  //  a0b1c2d   
  //  a0b1c  
  int pipes[job->total_process - 1][2];
  int i = 0;
  for ( ; i < job->total_process - 1; i++) {
    if (pipe(pipes[i]) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }
  struct process *process = job->first_process;
  i = 0;
  for ( ; i < job->total_process; i++, process = process->next_process) {
    int pid = fork();
    if (pid == 0) {
      if (i == 0) {
        if (input_file_descriptor != STDIN_FILENO) {
					dup2(input_file_descriptor, STDIN_FILENO);
					close(input_file_descriptor);
        }
      } else {
        // Read from previous pipe
        dup2(pipes[i - 1][0], STDIN_FILENO);
        close(pipes[i - 1][0]);
        close(pipes[i - 1][1]);
      }

      if (i == job->total_process - 1) {
        if (output_file_descriptor != STDOUT_FILENO) {
          dup2(output_file_descriptor, STDOUT_FILENO);
          close(output_file_descriptor);
        }
      } else {
        // Write to next pipe
        dup2(pipes[i][1], STDOUT_FILENO);
        close(pipes[i][0]);
        close(pipes[i][1]);
      }

      if (execvp(process->argv[0], process->argv)) {
        perror(process->argv[0]);
        exit(EXIT_FAILURE);
      }
    } else if (pid > 0) {
      if (i != 0) {
        close(pipes[i - 1][0]);
        close(pipes[i - 1][1]);
      }
    } else {
      perror("fork");
      exit(EXIT_FAILURE);
    }
  }

  if (input_file_descriptor != STDIN_FILENO) {
    close(input_file_descriptor);
  }
  if (output_file_descriptor != STDOUT_FILENO) {
    close(output_file_descriptor);
  }

  for ( ; i > 0; --i) {
    wait();
  }
}

int main(int argc, char *argv[]) {
  FILE *command_source = stdin;
  if (argc > 1) {
    if ((command_source = fopen(argv[1], "r")) == NULL) {
      perror(argv[1]);
      exit(EXIT_FAILURE);
    } 
  }

  while (TRUE) {
    if (command_source == stdin) {
      print_prompt();
    }
    char buffer[COMMAND_LENGTH_LIMIT];
    // puts it here to ensure prompt is printed before read input
    if (fgets(buffer, COMMAND_LENGTH_LIMIT, command_source) == NULL) {
      break;  
    }
    struct job job = (const struct job){NULL};
    parse(buffer, &job);
    execute_job(&job);
    //print_parse_result(&job);
    cleanup_job(&job);
  }

  fclose(command_source);
  return 0;
}
