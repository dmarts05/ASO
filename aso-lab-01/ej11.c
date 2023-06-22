#include <stdio.h>
#include <signal.h>

struct sigaction old_sa;
struct sigaction sa;

void handler(int sig)
{
  printf(" SIGINT received %d\n", sig);
  sigaction(SIGINT, &old_sa, NULL);
}

int main(void)
{
  sa.sa_handler = handler;
  sigaction(SIGINT, &sa, &old_sa);
  while (1)
  {
  }
  return 0;
}