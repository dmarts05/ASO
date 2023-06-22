#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

void imprimir_mapa_memoria()
{
    char cmd[25];
    sprintf(cmd, "cat /proc/%d/maps", getpid());
    system(cmd);
}

int main()
{
    imprimir_mapa_memoria();
    pid_t pid = fork();

    if (pid == 0)
    {
        printf("Hijo:\n");
        imprimir_mapa_memoria();
    }
    else
    {
        sleep(1);
        printf("Padre:\n");
        imprimir_mapa_memoria();
    }

    return 0;
}