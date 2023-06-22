#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void imprimirMapaMemoria()
{
  char cmd[25];

  sprintf(cmd, "cat /proc/%d/maps", getpid());
  system(cmd);
}

void funcion()
{
  char a[14000];
  imprimirMapaMemoria();
  printf("               a   %p   %12ld\n", &a, sizeof(a));
}

int main()
{
  imprimirMapaMemoria();
  funcion();
  imprimirMapaMemoria();

  return 0;
}