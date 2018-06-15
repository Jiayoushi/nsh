#include "parse.h"
#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

struct job *first_job;

void print_prompt() {
  printf("%c ", PROMPT);
  fflush(stdout);
}

void cleanup_job(struct job *job) {
  struct process *process = job->first_process;
  struct process *next_process = NULL;
  for ( ; process != NULL; process = next_process) {
    next_process = process->next_process;
    int index = 0;
    for ( ; process->argv[index] != NULL; index++) {
      free(process->argv[index]);
    }
    free(process);
  }
  free(job);
}

void cleanup_jobs() {
  struct job *job = first_job;
  struct job *next_job = NULL;
  for ( ; job != NULL; job = next_job) {
    next_job = job->next_job;
    cleanup_job(job);
  }
}

void execute_exit(struct job *job, struct job *last_job) {
  int exit_status = 0;
  if (job->first_process->argv[1] == NULL) {
    if (last_job == NULL) {
      exit_status = EXIT_SUCCESS;
    } else {
      exit_status = last_job->exit_status;
    }
  } else {
    if (job->first_process->argv[2] != NULL) {
      fprintf(stderr, "Usage: exit [status]\n");
      job->exit_status = 1;
      job->finished = TRUE;
      return ;
    }
    exit_status = atoi(job->first_process->argv[1]);
  }

  cleanup_jobs();
  exit(exit_status);
}

void execute_cd(struct job *job) {
  if (chdir(job->first_process->argv[1]) == -1) {
    perror("chdir");
    exit(EXIT_FAILURE);
  }
}

void execute_job(struct job *job, struct job *previous_job) {
  if (job->total_process == 0) {
    return;
  }

  if (strcmp(job->first_process->argv[0], "exit") == 0) {
    execute_exit(job, previous_job);
    return ;
  } else if (strcmp(job->first_process->argv[0], "cd") == 0) {
    execute_cd(job);
    return ;
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



void wait_background_job(struct job *job) {
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
          job->exit_status = WEXITSTATUS(status);
        } else {
					printf("process %s did not exit normally, exit status is not recorded\n", process->argv[0]);
        }
      } else {
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
    if (job->finished == TRUE || job->background == FALSE) {
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
    if (process->process_id == 0) {
      // Built-in command, no process id
    } else if (waitpid(process->process_id, &status, 0) == -1) {
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

struct job * allocate_job(struct job *job, struct job **first_job) {
  if (job == NULL) {
    job = (struct job *)malloc(sizeof(struct job));
    *first_job = job;
  } else {
    job->next_job = (struct job *)malloc(sizeof(struct job));
    job = job->next_job;
  }
  *job = (const struct job){NULL};
  return job;
}

void setup_input_source(int argc, char *argv[]) {
  if (argc == 2) {
    int input_source = 0;
    if ((input_source = open(argv[1], O_RDONLY, NULL)) == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    if (dup2(input_source, STDIN_FILENO) == -1) {
      perror("dup2");
      exit(EXIT_FAILURE);
    }
    if (close(input_source) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }
  }
}

int run_shell(int argc, char *argv[]) {
  setup_input_source(argc, argv); 

  struct job *job = NULL;
  struct job *previous_job = NULL;
  while (TRUE) {
    if (argc == 1) {
      print_prompt();
    }

    job = allocate_job(job, &first_job);
    int status = 0;
    if ((status = parse(job, previous_job)) == 1) {
      break;
    } 
    
    execute_job(job, previous_job);
    print_background_job(job);
    wait_foreground_job(job);
    wait_background_jobs(first_job);   
    previous_job = job;
  }
 
  cleanup_jobs(first_job);
  return 0;
}
