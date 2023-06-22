#include <stdio.h>
#include <signal.h>

struct sigaction sa;

void handler(int sig)
{
  printf(" SIGINT received %d\n", sig);
}

int main(void)
{
  sa.sa_flags = SA_RESETHAND;
  sa.sa_handler = handler;
  sigaction(SIGINT, &sa, NULL);
  while (1)
  {
  }
  return 0;
}