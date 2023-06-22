#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

void imprimirMapaMemoria()
{
  char cmd[25];

  sprintf(cmd, "cat /proc/%d/maps", getpid());
  system(cmd);
}

int main()
{
  double x = 1.5;
  double resultado = cos(x);
  imprimirMapaMemoria();

  return 0;
}