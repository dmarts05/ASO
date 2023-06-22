#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void load_program(int program_id)
{
  switch (program_id)
  {
  case 0:
    execl("/usr/bin/xeyes", (char *)0);
    break;
  case 1:
    execl("/usr/bin/xlogo", (char *)0);
    break;
  case 2:
    execl("/usr/bin/xload", (char *)0);
    break;
  case 3:
    execl("/usr/bin/xcalc", (char *)0);
    break;
  case 4:
    execl("/usr/bin/xclock", "xclock", "-update", "1", (char *)0);
    break;
  default:
    break;
  }
}

int main()
{

  pid_t current_pid;
  pid_t arr_pid[5];

  for (int i = 0; i < 5; i++)
  {
    current_pid = fork();

    if (current_pid == -1)
    {
      perror("Error en la llamada fork()");
    }
    else if (current_pid == 0)
    {
      // Cargamos el programa asociado al proceso hijo creado
      load_program(i);
    }
    else
    {
      // Almacenamos el pid del hijo creado en el array
      arr_pid[i] = current_pid;
    }
  }

  // Aquí solo llega el padre

  int status;
  current_pid = wait(&status);
  // El bucle se mantiene siempre que haya hijos
  while (current_pid != -1)
  {
    // Obtener índice del hijo que se ha cerrado
    int index = -1;
    for (int i = 0; i < 5; i++)
    {
      if (arr_pid[i] == current_pid)
      {
        index = i;
      }
    }

    // Volver a crear hijo para que ejecute el programa cerrado
    current_pid = fork();
    if (current_pid == -1)
    {
      perror("Error en la llamada fork()");
    }
    else if (current_pid == 0)
    {
      load_program(index);
    }
    else
    {
      arr_pid[index] = current_pid;
      current_pid = wait(&status);
    }
  }

  return 0;
}
