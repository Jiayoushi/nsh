#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define TRUE                  1
#define PROMPT                '?'


void print_prompt() {
  printf("%c ", PROMPT);
  fflush(stdout);
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

void execute_exit(struct job *job, struct job *last_job) {
  if (job->first_process->argv[1] == NULL) {
    if (last_job == NULL) {
      exit(EXIT_SUCCESS);
    } else {
      exit(last_job->exit_status);
    }
  } else {
    exit(atoi(job->first_process->argv[1]));
  }
}

void execute_job(struct job *job, struct job *previous_job) {
  if (job->total_process == 0) {
    return;
  }

  if (strcmp(job->first_process->argv[0], "exit") == 0) {
    execute_exit(job, previous_job);
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
      process->process_id = pid;
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
}

void cleanup_job(struct job *job) {
  struct process *process = job->first_process;
  struct process *next_process = NULL;
  for ( ; process != NULL; process = next_process) {
    next_process = process->next_process;
    free(process->argv);
    free(process);
  }
  free(job);
}

void cleanup_jobs(struct job *job) {
  struct job *next_job = NULL;
  for ( ; job != NULL; job = next_job) {
    next_job = job->next_job;
    cleanup_job(job);
  }
}

void wait_background_job(struct job *job) {
  printf("wait background job\n");
  struct process *process = job->first_process;
  int total_finished_process = 0;
  for ( ; process != NULL; process = process->next_process) {
    if (process->finished == TRUE) {
      total_finished_process++;
      continue;
    } else {
      int result = 0;
      int status = 0;
      if ((result = waitpid(process->process_id, &status, WNOHANG)) == -1) {
        perror("waitpid background job");
        exit(EXIT_FAILURE);
      } else if (result == process->process_id) {
        total_finished_process++;
        process->finished = TRUE;
        if (WIFEXITED(status)) {
          printf("set exit status\n");
          job->exit_status = WEXITSTATUS(status);
        } else {
					printf("process %s did not exit normally, exit status is not recorded\n", process->argv[0]);
        }
      } else {
        printf("result %d\n", result);
        return ;
      }
    }
  }

  if (total_finished_process == job->total_process) {
    printf("[-] %d\n", job->first_process->process_id);
    job->finished = TRUE;
  }
}

void wait_background_jobs(struct job *job) {
  for ( ; job != NULL; job = job->next_job) {
    if (job->finished == TRUE) {
      continue;
    }
    wait_background_job(job);
  } 
}

void wait_foreground_job(struct job *job) {
  if (job->background == TRUE || job->finished == TRUE) {
    return ;
  }

  struct process *process = job->first_process;
  for ( ; process != NULL; process = process->next_process) {
    int status = 0;
    if (waitpid(process->process_id, &status, 0) == -1) {
      perror("waitpid foreground job");
      exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) {
      job->exit_status = WEXITSTATUS(status); 
    } else {
      printf("process %s did not exit normally, exit status is not recorded\n", process->argv[0]);
    }
    process->finished = TRUE;
  }
  job->finished = TRUE;
}

void print_background_job(struct job *job) {
  if (job->background != TRUE) {
    return ;
  }

  printf("[+] %d\n", job->first_process->process_id);
}

int main(int argc, char *argv[]) {
  FILE *command_source = stdin;
  if (argc > 1) {
    if ((command_source = fopen(argv[1], "r")) == NULL) {
      perror(argv[1]);
      exit(EXIT_FAILURE);
    } 
  }

  struct job *job = NULL;
  struct job *first_job = NULL;
  struct job *previous_job = NULL;
  while (TRUE) {
    wait_background_jobs(first_job);

    if (command_source == stdin) {
      print_prompt();
    }
    
    if (job == NULL) {
      job = (struct job *)malloc(sizeof(struct job));
      first_job = job;
    } else {
      job->next_job = (struct job *)malloc(sizeof(struct job));
      job = job->next_job;
    }
    *job = (const struct job){NULL};

    if (fgets(job->command, COMMAND_LENGTH_LIMIT, command_source) == NULL) {
      break;  
    }
    *strchr(job->command, '\n') = '\0';
    parse(job->command, job);
    execute_job(job, previous_job);
    //print_parse_result(job);
    print_background_job(job);
    wait_foreground_job(job);
    previous_job = job;
  }

  cleanup_jobs(first_job);
  fclose(command_source);
  return 0;
}
