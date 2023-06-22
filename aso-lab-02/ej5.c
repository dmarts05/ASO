#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void imprimirMapaMemoria()
{
  char cmd[25];

  sprintf(cmd, "cat /proc/%d/maps", getpid());
  system(cmd);
}

void *hilo_funcion()
{
  int var_local[2] = {1, 2};
  printf("Dentro del hilo: la variable local está en la dirección %p y tiene un tamaño de %lu bytes\n", var_local, sizeof(var_local));
}

int main()
{
  pthread_t hilo;
  pthread_t hilo2;

  imprimirMapaMemoria();

  if (pthread_create(&hilo, NULL, hilo_funcion, NULL) != 0)
  {
    printf("Error al crear el hilo\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&hilo2, NULL, hilo_funcion, NULL) != 0)
  {
    printf("Error al crear el hilo 2\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_join(hilo, NULL) != 0)
  {
    printf("Error al esperar por el hilo\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_join(hilo2, NULL) != 0)
  {
    printf("Error al esperar por el hilo 2\n");
    exit(EXIT_FAILURE);
  }

  imprimirMapaMemoria();

  return 0;
}