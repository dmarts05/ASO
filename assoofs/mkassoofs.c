#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "assoofs.h"

#define WELCOMEFILE_DATABLOCK_NUMBER (ASSOOFS_LAST_RESERVED_BLOCK + 1)
#define WELCOMEFILE_INODE_NUMBER (ASSOOFS_LAST_RESERVED_INODE + 1)

// **************************
// Declaraciones de funciones
// **************************

/**
 * Escribe el superbloque del sistema de archivos
 * en el primer bloque del dispositivo
 *
 * @param fd El descriptor de archivo del dispositivo
 *
 * @return 0 si todo salió bien, -1 en caso contrario
 */
static int write_superblock(int fd);

/**
 * Almacena el inodo del directorio raíz en el almacén de inodos
 *
 * @param fd El descriptor de archivo del dispositivo
 *
 * @return 0 si todo salió bien, -1 en caso contrario
 */
static int write_root_inode(int fd);

/**
 * Almacena el inodo de welcomefile en el almacén de inodos
 *
 * @param fd El descriptor de archivo del dispositivo
 * @param i Puntero al inodo de welcomefile
 *
 * @return 0 si todo salió bien, -1 en caso contrario
 */
static int write_welcome_inode(int fd, const struct assoofs_inode_info *i);

/**
 * Escribe una entrada de directorio en un descriptor de archivo
 *
 * @param fd El descriptor de archivo del dispositivo
 * @param record Puntero a la entrada de directorio
 *
 * @return 0 si todo salió bien, -1 en caso contrario
 */
int write_dirent(int fd, const struct assoofs_dir_record_entry *record);

/**
 * Escribe un bloque en un descriptor de archivo
 *
 * @param fd El descriptor de archivo del dispositivo
 * @param block Puntero al bloque
 * @param len Tamaño del bloque en bytes
 *
 * @return 0 si todo salió bien, -1 en caso contrario
 */
int write_block(int fd, char *block, size_t len);

// +++++++++++++++++++++++++
// Definiciones de funciones
// +++++++++++++++++++++++++

static int write_superblock(int fd)
{
    // Crear el superbloque
    struct assoofs_super_block_info sb = {
        .version = 1,
        .magic = ASSOOFS_MAGIC,
        .block_size = ASSOOFS_DEFAULT_BLOCK_SIZE,
        .inodes_count = WELCOMEFILE_INODE_NUMBER,
        // Bloques libres = Todos los bloques - (superbloque + almacenamiento de inodos + directorio raíz + welcomefile)
        .free_blocks = (~0) & ~(15),
    };

    // ret representa el número de bytes escritos
    ssize_t ret;

    // Escribe el superbloque en el primer bloque
    ret = write(fd, &sb, sizeof(sb));
    if (ret != ASSOOFS_DEFAULT_BLOCK_SIZE)
    {
        printf("Bytes written [%d] are not equal to the default block size.\n", (int)ret);
        return -1;
    }

    printf("Super block written succesfully.\n");
    return 0;
}

static int write_root_inode(int fd)
{
    // ret representa el número de bytes escritos
    ssize_t ret;

    // Crear el inodo del directorio raíz
    struct assoofs_inode_info root_inode;

    root_inode.mode = S_IFDIR;
    root_inode.inode_no = ASSOOFS_ROOTDIR_INODE_NUMBER;
    root_inode.data_block_number = ASSOOFS_ROOTDIR_BLOCK_NUMBER;
    root_inode.dir_children_count = 1;

    // Escribe el inodo del directorio raíz en el almacén de inodos
    ret = write(fd, &root_inode, sizeof(root_inode));

    if (ret != sizeof(root_inode))
    {
        printf("The inode store was not written properly.\n");
        return -1;
    }

    printf("root directory inode written succesfully.\n");
    return 0;
}

static int write_welcome_inode(int fd, const struct assoofs_inode_info *i)
{
    // nbytes representa el número de bytes necesario de padding para alinear el siguiente inodo
    off_t nbytes;
    // ret representa el número de bytes escritos
    ssize_t ret;

    // Escribe el inodo de welcomefile en el almacén de inodos
    ret = write(fd, i, sizeof(*i));
    if (ret != sizeof(*i))
    {
        printf("The welcomefile inode was not written properly.\n");
        return -1;
    }
    printf("welcomefile inode written succesfully.\n");

    // Esto asegura que el tamaño total de dos inodos más los bytes de padding coincida con el tamaño del bloque.
    nbytes = ASSOOFS_DEFAULT_BLOCK_SIZE - (sizeof(*i) * 2);
    // Mueve el puntero de archivo hacia adelante por nbytes (padding) desde la posición actual (SEEK_CUR)
    ret = lseek(fd, nbytes, SEEK_CUR);

    // (off_t)-1 representa un error en lseek. -1 es casteado a off_t
    if (ret == (off_t)-1)
    {
        printf("The padding bytes were not written properly.\n");
        return -1;
    }

    printf("inode store padding bytes (after two inodes) written sucessfully.\n");
    return 0;
}

int write_dirent(int fd, const struct assoofs_dir_record_entry *record)
{
    // nbytes representa el número de bytes necesarios para escribir la entrada de directorio
    ssize_t nbytes = sizeof(*record);
    // ret representa el número de bytes escritos
    ssize_t ret;

    // Escribe la entrada de directorio en el descriptor de archivo
    ret = write(fd, record, nbytes);
    if (ret != nbytes)
    {
        printf("Writing the rootdirectory datablock (name+inode_no pair for welcomefile) has failed.\n");
        return -1;
    }
    printf("root directory datablocks (name+inode_no pair for welcomefile) written succesfully.\n");

    // nbytes representa el número de bytes necesario de padding para alinear el siguiente inodo
    nbytes = ASSOOFS_DEFAULT_BLOCK_SIZE - sizeof(*record);
    // Mueve el puntero de archivo hacia adelante por nbytes (padding) desde la posición actual (SEEK_CUR)
    ret = lseek(fd, nbytes, SEEK_CUR);
    if (ret == (off_t)-1)
    {
        printf("Writing the padding for rootdirectory children datablock has failed.\n");
        return -1;
    }
    printf("Padding after the rootdirectory children written succesfully.\n");
    return 0;
}

int write_block(int fd, char *block, size_t len)
{
    // ret representa el número de bytes escritos
    ssize_t ret;

    // Escribe el bloque en el descriptor de archivo
    ret = write(fd, block, len);
    if (ret != len)
    {
        printf("Writing file body has failed.\n");
        return -1;
    }
    printf("block has been written succesfully.\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    ssize_t ret;
    char welcomefile_body[] = "Hola mundo, os saludo desde un sistema de ficheros ASSOOFS.\n";

    // Define la información del inodo de welcomefile
    struct assoofs_inode_info welcome = {
        .mode = S_IFREG,
        .inode_no = WELCOMEFILE_INODE_NUMBER,
        .data_block_number = WELCOMEFILE_DATABLOCK_NUMBER,
        .file_size = sizeof(welcomefile_body),
    };

    // Define la entrada de directorio para welcomefile
    struct assoofs_dir_record_entry record = {
        .filename = "README.txt",
        .inode_no = WELCOMEFILE_INODE_NUMBER,
    };

    // Comprueba que el número de argumentos sea correcto
    if (argc != 2)
    {
        printf("Usage: mkassoofs <device>\n");
        return -1;
    }

    // Abre el dispositivo especificado (argumento de línea de comandos) en modo lectura/escritura
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        perror("Error opening the device");
        return -1;
    }

    // Inicializa ret a 1 (indicando un error) para el bucle do-while
    ret = 1;
    do
    {
        // Escribe el superbloque
        if (write_superblock(fd))
            break;

        // Escribe el inodo raíz
        if (write_root_inode(fd))
            break;

        // Escribe el inodo de welcomefile
        if (write_welcome_inode(fd, &welcome))
            break;

        // Escribe la entrada de directorio para welcomefile
        if (write_dirent(fd, &record))
            break;

        // Escribir el contenido de welcomefile
        if (write_block(fd, welcomefile_body, welcome.file_size))
            break;

        // Si todo salió bien, establece ret a 0
        ret = 0;
    } while (0);

    // Cierra el descriptor de archivo
    close(fd);

    // Devuelve 0 si todo salió bien, -1 en caso contrario
    return ret;
}
