#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
  printf("Este es el PID del programa: %d\n", getpid());
  execl("/bin/xeyes", (char *)0);
  printf("Este es el PID del programa: %d\n", getpid());
  return 0;
}