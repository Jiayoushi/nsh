typedef struct process {
  struct process *next_process;     // this_process | next_process
  char *name; // Name of the process
  char *input_source;
  char *output_source;
} process;

typedef struct job {
  process *process_list;
  int size_of_process_list;;
} job;

int parse(const char *command, job job);

