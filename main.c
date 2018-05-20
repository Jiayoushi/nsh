#include "parse.h"

#include <stdio.h>


#define COMMAND_LENGTH_LIMIT  1024
#define TRUE                  1
#define PROMPT                '?'


void print_prompt() {
  printf("%c ", PROMPT);
}

void print_parse_result(job job) {

}

int main() {
  while (TRUE) {
    print_prompt();
    char buffer[COMMAND_LENGTH_LIMIT];
    if (fgets(buffer, COMMAND_LENGTH_LIMIT, stdin) == NULL) {
      break;  
    }
    job job;
    parse(buffer, job);
    print_parse_result(job);
  }

  return 0;
}
