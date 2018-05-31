#include <sys/types.h>

#define COMMAND_LENGTH_LIMIT  1024
#define MAX_FILENAME_LENGTH        255
#define DEFAULT_ARGV_SIZE            2
#define TRUE                         1
#define FALSE                        0

struct process {
  struct process *next_process;     
  char **argv;
  int finished;
  pid_t process_id;
};

struct job {
  struct job *next_job;
  struct process *first_process;
  int total_process;
  int background;
  char command[COMMAND_LENGTH_LIMIT]; // TODO: Should be changed later for dynamic length
  int input_redirect_mode;        // 0 for non, 1 for <, 2 for <<
  int output_redirect_mode;       
  char input_redirect_filename[MAX_FILENAME_LENGTH];
  char output_redirect_filename[MAX_FILENAME_LENGTH];
};

int parse(char *command, struct job *job);

