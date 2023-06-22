#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void imprimirMapaMemoria()
{
  char cmd[25];

  sprintf(cmd, "cat /proc/%d/maps", getpid());
  system(cmd);
}

int main()
{
  int pid = fork();

  if (pid == 0)
  {
    imprimirMapaMemoria();
    execl("/usr/bin/cat", "cat", "/proc/self/maps", NULL);
    imprimirMapaMemoria();
  }
  else
  {
    wait(NULL);
  }

  return 0;
}