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
    int padre = getppid();
    int yo = getpid();
    int abuelo;
    read(tubo[0], &abuelo, sizeof(abuelo));

    printf("[H] Mi padre=%d, yo=%d, mi abuelo=%d\n", padre, yo, abuelo);
  }
  else
  {
    // Padre
    int padre = getppid();
    int yo = getpid();
    int hijo = pid;
    write(tubo[1], &padre, sizeof(padre));

    printf("[H] Mi padre=%d, yo=%d, mi hijo=%d\n", padre, yo, hijo);
  }

  return 0;
}
