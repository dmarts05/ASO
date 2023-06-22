#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
  int og = open(argv[1], O_RDONLY);
  if (og)
  {
    int cp = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    char buffer[4096];
    int text;

    do
    {
      text = read(og, buffer, sizeof(buffer));
      write(cp, buffer, text);

    } while (text == sizeof(buffer));

    close(cp);
    close(og);
  }
  else
  {
    perror("¡El arhivo especificado no existe!\n");
  }
  return 0;
}
