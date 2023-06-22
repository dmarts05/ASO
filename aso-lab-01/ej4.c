#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
  int pid = fork();

  if (pid == 0)
  {
    // Hijo
    sleep(1);
    printf("[H] ppid = %5d, pid = %5d\n", getppid(), getpid());
    exit(33);
  }
  else
  {
    // Padre
    printf("[P] ppid = %5d, pid = %5d, H = %5d\n", getppid(), getpid(), pid);
    int status;
    waitpid(pid, &status, 0);
    int returned = WEXITSTATUS(status);
    printf("[P] el proceso pid=%d acaba de terminar con estado %d y retorna %d", pid, status, returned);
  }
  return 0;
}