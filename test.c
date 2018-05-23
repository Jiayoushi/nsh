#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
  char str[] = "ls\0-l";
  char * x[5];
  x[0] = (char *)&str[0];
  x[1] = (char *)&str[3];
  x[2] = (char *)NULL;
  execvp(x[0], x);
  return 0;
}
