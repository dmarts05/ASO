#include <stdio.h>
#include <signal.h>

struct sigaction sa;

// void handler(int sig)
// {
//   printf(" SIGINT received %d\n", sig);
// }

int main(void)
{
  sa.sa_handler = SIG_IGN;
  sigaction(SIGINT, &sa, NULL);
  while (1)
  {
  }
  return 0;
}