#include <stdio.h>
#include <signal.h>

struct sigaction first_sa;
struct sigaction second_sa;
struct sigaction old_sa;

void handler1(int sig)
{
  printf("¡Primer mensaje!\n");
  sigaction(SIGINT, &second_sa, &old_sa);
}

void handler2(int sig)
{
  printf("¡Segundo mensaje!\n");
  sigaction(SIGINT, &old_sa, NULL);
}

int main(void)
{
  first_sa.sa_handler = handler1;
  second_sa.sa_handler = handler2;
  sigaction(SIGINT, &first_sa, NULL);
  while (1)
  {
  }
  return 0;
}