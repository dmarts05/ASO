#include <stdio.h>
#include <signal.h>
#include <unistd.h>

struct sigaction sa;
int seconds;
void tic(int i)
{
  seconds++;
  alarm(1);
}

int main(void)
{
  seconds = 0;
  sa.sa_handler = tic;
  sigaction(SIGALRM, &sa, NULL);
  alarm(1);

  while (1)
  {
    pause();
    printf("%d\n", seconds);
  }

  return 0;
}