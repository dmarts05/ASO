#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
  int pid = fork();

  if (pid == 0)
  {
    // Hijo
    sleep(1);
    printf("[H] ppid = %5d, pid = %5d\n", getppid(), getpid());
  }
  else
  {
    // Padre
    printf("[P] ppid = %5d, pid = %5d, H = %5d\n", getppid(), getpid(), pid);
    int status;
    wait(&status);
    printf("[P] el proceso pid=%d acaba de terminar con estado %d", pid, status);
  }
  return 0;
}