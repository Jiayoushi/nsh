#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <sys/wait.h>

extern int errno;

void print_info() {
  printf("ruid %d\neuid %d\npid %d\nrgid %d\negid %d\n\n",
         getuid(), geteuid(), getpid(), getgid(), getegid());
}


int main() {
  print_info();
  int pid = 0;
  char *s[2] = {"ls", NULL};
  if ((pid = fork()) == 0) {
    execvp("ls", s);
    exit(0);
  }
   
  /*
  int status;
  int result = 0;
 
  while (1) {
		if ((result = waitpid(pid, &status, WNOHANG)) < 0) {
			perror("second waitpid");
			exit(EXIT_FAILURE);
		} else if (result == pid){
			printf("I got you !\n");
			exit(0);
		} else {
			printf("not returned yet\n");
		}
  }*/

  return 0;
}
