#include <sys/types.h>

#define COMMAND_LENGTH_LIMIT  1024
#define MAX_FILENAME_LENGTH        255
#define DEFAULT_ARGV_SIZE            2
#define TRUE                         1
#define FALSE                        0
#define MAX_ARGUMENTS          32
#define MAX_ARGUMENT_LENGTH    255

struct process {
  struct process *next_process;     
  char *argv[MAX_ARGUMENTS];
  int finished;
  pid_t process_id;
};

struct job {
  struct job *next_job;
  struct process *first_process;
  int total_process;
  int finished;
  int background;
  int exit_status;
  int input_redirect_mode;        // 0 for non, 1 for <, 2 for <<
  int output_redirect_mode;       
  char input_redirect_filename[MAX_FILENAME_LENGTH];
  char output_redirect_filename[MAX_FILENAME_LENGTH];
};

// Return 0 for sucess, 1 for no input received
int parse(struct job *job, struct job *previous_job);

