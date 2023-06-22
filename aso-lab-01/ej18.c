#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  char msg[10] = "Hola mundo";
  char received_msg[10];
  int num_bytes;
  int tube[2];
  pipe(tube);

  int pid = fork();

  if (pid == 0)
  {
    write(tube[1], msg, sizeof(msg));
  }
  else
  {
    num_bytes = read(tube[0], received_msg, sizeof(received_msg));
    printf("Contenido del mensaje: %s\n", received_msg);
  }

  return 0;
}
