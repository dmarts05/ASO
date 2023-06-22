#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <math.h>
#include <gnu/lib-names.h>
#include <unistd.h>
#include <errno.h>

void imprimirMapaMemoria()
{
  char cmd[25];

  sprintf(cmd, "cat /proc/%d/maps", getpid());
  system(cmd);
}

int main()
{
  printf("Antes de dlopen:\n");
  imprimirMapaMemoria();

  void *handle;
  double (*cosine)(double);
  char *error;

  handle = dlopen(LIBM_SO, RTLD_LAZY);
  if (!handle)
  {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  cosine = dlsym(handle, "cos");
  error = dlerror();
  if (error != NULL)
  {
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
  }

  printf("Después de dlopen:\n");
  imprimirMapaMemoria();

  double result = (*cosine)(0.0);

  dlclose(handle);

  printf("Después de dlclose:\n");
  imprimirMapaMemoria();

  return 0;
}