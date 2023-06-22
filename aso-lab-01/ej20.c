#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int tubo[2];
  pipe(tubo);
  int pid = fork();

  if (pid == 0)
  {
    // Hijo
    close(tubo[1]);
    dup2(tubo[0], 0);
    execl("/usr/bin/wc", (char *)0);
  }
  else
  {
    // Padre
    close(tubo[0]);
    dup2(tubo[1], 1);
    execl("/usr/bin/ls", "ls", "-l", (char *)0);
  }

  return 0;
}
