#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
  int file_desc = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
  char text[11] = "Hola Mundo\n";
  write(file_desc, text, sizeof(text));
  close(file_desc);
  return 0;
}
