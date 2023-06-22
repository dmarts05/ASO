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
  char *a;
  a = malloc(sizeof(char) * 14000);
  imprimirMapaMemoria();
  printf("               a   %p   %12ld\n", &a, sizeof(a));
  printf("               a (heap)   %p   %12ld\n", a, sizeof(a));
  free(a);
}

int main()
{
  imprimirMapaMemoria();
  funcion();
  imprimirMapaMemoria();

  return 0;
}