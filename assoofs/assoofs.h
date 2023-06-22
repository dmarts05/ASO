#define ASSOOFS_MAGIC 0x20200406
#define ASSOOFS_DEFAULT_BLOCK_SIZE 4096
#define ASSOOFS_FILENAME_MAXLEN 255
#define ASSOOFS_LAST_RESERVED_BLOCK ASSOOFS_ROOTDIR_BLOCK_NUMBER
#define ASSOOFS_LAST_RESERVED_INODE ASSOOFS_ROOTDIR_INODE_NUMBER

const int ASSOOFS_SUPERBLOCK_BLOCK_NUMBER = 0;
const int ASSOOFS_INODESTORE_BLOCK_NUMBER = 1;
const int ASSOOFS_ROOTDIR_BLOCK_NUMBER = 2;
const int ASSOOFS_ROOTDIR_INODE_NUMBER = 1;
const int ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED = 64;

/**
 * Representa la información del superbloque del sistema de archivos
 *
 * @param version La versión del sistema de archivos
 * @param magic El número mágico del sistema de archivos
 * @param block_size El tamaño de bloque del sistema de archivos
 * @param inodes_count El número de inodos en el sistema de archivos
 * @param free_blocks El número de bloques libres en el sistema de archivos
 * @param padding Relleno adicional para que coincida con el tamaño de bloque (4096 bytes)
 */
struct assoofs_super_block_info
{
    uint64_t version;
    uint64_t magic;
    uint64_t block_size;
    uint64_t inodes_count;
    uint64_t free_blocks;

    char padding[4056];
};

/**
 * Representa una entrada de directorio en el sistema de archivos
 *
 * @param filename El nombre del archivo
 * @param inode_no El número de inodo del archivo
 */
struct assoofs_dir_record_entry
{
    char filename[ASSOOFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

/**
 * Representa la información de un inodo en el sistema de archivos
 *
 * @param mode El modo del archivo (directorio o archivo)
 * @param inode_no El número de inodo del archivo
 * @param data_block_number El número de bloque de datos del archivo (donde se almacenan los datos)
 * @param file_size El tamaño del archivo (si el inodo describe un archivo)
 * @param dir_children_count El número de hijos del directorio (si el inodo describe un directorio)
 */
struct assoofs_inode_info
{
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_number;

    union
    {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};
