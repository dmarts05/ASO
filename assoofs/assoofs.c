#include <linux/module.h>      /* Needed by all modules */
#include <linux/kernel.h>      /* Needed for KERN_INFO  */
#include <linux/init.h>        /* Needed for the macros */
#include <linux/fs.h>          /* libfs stuff           */
#include <linux/buffer_head.h> /* buffer_head           */
#include <linux/slab.h>        /* kmem_cache            */
#include "assoofs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Angel Manuel Guerrero Higueras");

// ******************************
// Declaración de global de caché
// ******************************

static struct kmem_cache *assoofs_inode_cache;

// ***********************************
// Declaración de funciones auxiliares
// ***********************************

/**
 * Obtiene la información persistente de un inodo específico.
 *
 * @param sb Puntero al superbloque que contiene el sistema de archivos assoofs.
 * @param inode_no Número de inodo del cual se quiere obtener la información persistente.
 *
 * @return Devuelve un puntero a un struct assoofs_inode_info que contiene la información persistente del inodo encontrado, o NULL si el inodo no se encuentra o hay un error.
 */

struct assoofs_inode_info *assoofs_get_inode_info(struct super_block *sb, uint64_t inode_no);

/**
 * Obtiene un inodo existente del sistema de archivos assoofs.
 *
 * @param sb Puntero al superbloque que contiene el sistema de archivos assoofs.
 * @param ino Número de inodo del cual se quiere obtener el inodo.
 *
 * @return Devuelve un puntero a un struct inode que contiene el inodo encontrado, o NULL si el inodo no se encuentra o hay un error.
 */
static struct inode *assoofs_get_inode(struct super_block *sb, int ino);

/**
 * Función para actualizar la información persistente del superbloque en el dispositivo de bloques.
 *
 * @param vsb Puntero al superbloque del sistema de archivos.
 */
void assoofs_save_sb_info(struct super_block *vsb);

/**
 * Función para obtener un bloque libre en el dispositivo de bloques.
 * Busca el primer bloque libre en el mapa de bits de bloques libres del superbloque.
 * Si encuentra un bloque libre, lo marca como ocupado en el mapa de bits y devuelve su número.
 *
 * @param sb Superbloque del sistema de archivos.
 * @param block Puntero a un entero sin signo de 64 bits donde se almacenará el número de bloque libre.
 *
 * @return 0 si se encuentra un bloque libre, un valor negativo en caso contrario.
 */
int assoofs_sb_get_a_freeblock(struct super_block *sb, uint64_t *block);

/**
 * Función que añade un nuevo inodo al almacén de inodos.
 *
 * @param sb Puntero al superbloque del sistema de ficheros.
 * @param inode Puntero al inodo que se va a añadir.
 */
void assoofs_add_inode_info(struct super_block *sb, struct assoofs_inode_info *inode);

/**
 * Función que obtiene un puntero a la información persistente de un inodo específico.
 *
 * @param sb Puntero al superbloque del sistema de ficheros.
 * @param start Puntero al primer inodo del almacén de inodos.
 * @param search Puntero al inodo que se quiere buscar.
 *
 * @return Devuelve un puntero a la información persistente del inodo encontrado, o NULL si el inodo no se encuentra o hay un error.
 */
struct assoofs_inode_info *assoofs_search_inode_info(struct super_block *sb, struct assoofs_inode_info *start, struct assoofs_inode_info *search);

/**
 * Función que actualiza la información persistente de un inodo en el dispositivo de bloques.
 *
 * @param sb Puntero al superbloque del sistema de ficheros.
 * @param inode_info Puntero a la información persistente del inodo que se va a actualizar.
 *
 * @return 0 si se actualiza correctamente, un valor negativo en caso contrario.
 */
int assoofs_save_inode_info(struct super_block *sb, struct assoofs_inode_info *inode_info);

// *************************************************************
// Declaración de funciones y structs de operaciones de ficheros
// *************************************************************

/**
 * Función que permite leer el contenido de un fichero.
 *
 * @param filp Puntero al archivo que se va a leer.
 * @param buf Puntero al buffer donde se almacenará el contenido del fichero.
 * @param len Tamaño del buffer.
 * @param ppos Puntero a la posición actual del fichero.
 *
 * @return Número de bytes leídos, o un valor negativo en caso de error.
 */
ssize_t assoofs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos);

/**
 * Función que permite escribir en un fichero.
 *
 * @param filp Puntero al archivo que se va a escribir.
 * @param buf Puntero al buffer que contiene los datos que se van a escribir.
 * @param len Tamaño del buffer.
 * @param ppos Puntero a la posición actual del fichero.
 */
ssize_t assoofs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos);

const struct file_operations assoofs_file_operations = {
    .read = assoofs_read,
    .write = assoofs_write,
};

// ****************************************************************
// Declaración de funciones y structs de operaciones de directorios
// ****************************************************************

/**
 * Función que permite mostrar el contenido de un directorio.
 *
 * @param filp Puntero al archivo que representa el directorio.
 * @param ctx Puntero al contexto del directorio.
 *
 * @return 0 si se muestra correctamente, un valor negativo en caso contrario.
 */
static int assoofs_iterate(struct file *filp, struct dir_context *ctx);

const struct file_operations assoofs_dir_operations = {
    .owner = THIS_MODULE,
    .iterate = assoofs_iterate,
};

// ***********************************************************
// Declaración de funciones y structs de operaciones de inodos
// ***********************************************************

/**
 * Busca una entrada de directorio específica en el directorio padre.
 * @param parent_inode Puntero al inodo del directorio padre.
 * @param child_dentry Puntero al dentry que representa la entrada de directorio a buscar.
 * @param flags Bandera(s) adicionales para la búsqueda (no utilizado en esta implementación).
 *
 * @return Devuelve siempre NULL.
 */
struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);

/**
 * Crea un nuevo inodo en el directorio especificado.
 *
 * @param mnt_userns Puntero al espacio de nombres del usuario.
 * @param dir Puntero al inodo del directorio donde se creará el nuevo inodo.
 * @param dentry Puntero a la entrada del directorio padre para el nuevo archivo.
 * @param mode Permisos del nuevo archivo.
 * @param excl Bandera que indica si el archivo debe ser creado en modo exclusivo (no utilizado en esta implementación).
 *
 * @return 0 si el archivo se creó correctamente, un valor negativo en caso contrario.
 */
static int assoofs_create(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);

/**
 * Crea un nuevo directorio en el directorio especificado.
 *
 * @param mnt_userns Puntero al espacio de nombres del usuario.
 * @param dir Puntero al inodo del directorio donde se creará el nuevo directorio.
 * @param dentry Puntero a la entrada del directorio padre para el nuevo directorio (nombre del directorio)
 * @param mode Permisos del nuevo directorio.
 *
 * @return 0 si el directorio se creó correctamente, un valor negativo en caso contrario.
 */
static int assoofs_mkdir(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode);

static struct inode_operations assoofs_inode_ops = {
    .create = assoofs_create,
    .lookup = assoofs_lookup,
    .mkdir = assoofs_mkdir,
};

// ****************************************************************
// Declaración de funciones y structs de operaciones de superbloque
// ****************************************************************

/**
 * Función que libera la memoria asociada a un inodo.
 *
 * @param inode Puntero al inodo que se va a liberar.
 *
 * @return 0 si se libera correctamente, un valor negativo en caso contrario.
 */
int assoofs_destroy_inode(struct inode *inode);

static const struct super_operations assoofs_sops = {
    .drop_inode = assoofs_destroy_inode,
};

/**
 * Rellena el superbloque para el sistema de archivos assoofs.
 *
 * @param sb Puntero al superbloque a rellenar.
 * @param data Puntero a los datos específicos del sistema de archivos.
 * @param silent Indica si se deben imprimir mensajes de error o no.
 *
 * @return 0 si todo ha ido bien, un número negativo en caso de error.
 */
int assoofs_fill_super(struct super_block *sb, void *data, int silent);

// ***********************************
// Declaración de funciones de montaje
// ***********************************

/**
 * Función que permite montar dispositivos del sistema de archivos assoofs.
 *
 * @param fs_type Estructura file_system_type que contiene información sobre el sistema de archivos.
 * @param flags Flags adicionales de montaje.
 * @param dev_name cadena de caracteres con el nombre del dispositivo.
 * @param data Puntero a una cadena de caracteres con información adicional.
 *
 * @return Puntero a una estructura dentry que representa el directorio raíz del sistema de archivos montado.
 */
static struct dentry *assoofs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

// **************************************************************************
// Declaración de funciones y structs de inicialización y descarga del módulo
// **************************************************************************

/**
 * Definición de la estructura file_system_type utilizada para registrar y gestionar sistemas de archivos en el kernel de Linux.
 */
static struct file_system_type assoofs_type = {
    .owner = THIS_MODULE,        // Propietario del módulo
    .name = "assoofs",           // Nombre del sistema de archivos
    .mount = assoofs_mount,      // Función de montaje del sistema de archivos
    .kill_sb = kill_block_super, // Función para eliminar el superbloque
};

/**
 * Función de inicialización del módulo del sistema de archivos assoofs.
 * Se ejecuta cuando se carga el módulo en el kernel.
 * Registra el nuevo sistema de archivos en el kernel utilizando la función register_filesystem.
 *
 * @return 0 si la inicialización se realiza correctamente, un valor negativo en caso de error.
 */
static int __init assoofs_init(void);

/**
 * Función de salida del módulo del sistema de archivos assoofs.
 * Se ejecuta cuando se borra el módulo del kernel.
 * Elimina la información del sistema de archivos del kernel utilizando la función unregister_filesystem.
 */
static void __exit assoofs_exit(void);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// +++++++++++++++++++++++++++++++++
// Definicón de funciones auxiliares
// +++++++++++++++++++++++++++++++++

struct assoofs_inode_info *assoofs_get_inode_info(struct super_block *sb, uint64_t inode_no)
{
    // Declaración de variables (ISO C90)
    int i;
    struct assoofs_super_block_info *afs_sb;
    struct assoofs_inode_info *buffer;
    struct assoofs_inode_info *inode_info;
    struct buffer_head *bh;

    printk(KERN_INFO "assoofs_get_inode_info: request\n");

    // 1. Leer el bloque que contiene el almacén de inodos del dispositivo de bloques
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    // sb_bread se utiliza aquí para leer el almacén de inodos del dispositivo de bloques (el bloque 1)
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_get_inode_info: reading the inode store failed\n");
        return NULL;
    }
    // Para acceder a los campos del bloque, primero hay que asignar bh->b_data a un puntero de tipo assoofs_inode_info
    inode_info = (struct assoofs_inode_info *)bh->b_data;

    // 2. Recorrer el almacén de inodos en busca del inodo cuya información se quiere obtener (inode_no)
    // Declaramos un puntero con la información persistente del superbloque
    afs_sb = sb->s_fs_info;
    // Preparamos un buffer para copiar la información del inodo
    buffer = NULL;
    for (i = 0; i < afs_sb->inodes_count; i++)
    {
        if (inode_info->inode_no == inode_no)
        {
            // Hemos encontrado el inodo que buscábamos
            // Copiamos la información persistente del inodo en el buffer
            buffer = kmalloc(sizeof(struct assoofs_inode_info), GFP_KERNEL);
            memcpy(buffer, inode_info, sizeof(*buffer));
            break;
        }
        inode_info++;
    }

    // Ya podemos liberar el buffer_head con brelse
    brelse(bh);

    return buffer;
};

static struct inode *assoofs_get_inode(struct super_block *sb, int ino)
{
    // Declaración de variables (ISO C90)
    struct assoofs_inode_info *inode_info;
    struct inode *inode;

    printk(KERN_INFO "assoofs_get_inode: request\n");

    // 1. Obtenemos la información persistente del inodo ino
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info = assoofs_get_inode_info(sb, ino);
    if (!inode_info)
    {
        printk(KERN_ERR "assoofs_get_inode_info: Inode not found\n");
        return NULL;
    }

    // 2. Creamos un nuevo inodo y asignamos los campos correspondientes
    // Inicializamos el inodo
    inode = new_inode(sb);
    // Asignamos el número de inodo
    inode->i_ino = ino;
    // Asignamos el superbloque
    inode->i_sb = sb;
    // Asignamos las operaciones sobre inodos
    inode->i_op = &assoofs_inode_ops;

    // Determinamos si el inodo es un fichero o un directorio y asignamos la operación correspondiente
    if (S_ISDIR(inode_info->mode))
        inode->i_fop = &assoofs_dir_operations;
    else if (S_ISREG(inode_info->mode))
        inode->i_fop = &assoofs_file_operations;
    else
    {
        printk(KERN_ERR "assoofs_get_inode: Unknown inode type. Neither a directory nor a file.");
        return NULL;
    }

    // Asignar fecha del sistema a los campos i_atime, i_mtime, i_ctime
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

    // Guardar la información persistente del inodo en el campo i_private
    inode->i_private = inode_info;

    // 3. Devolver el inodo recién creado
    return inode;
}

void assoofs_save_sb_info(struct super_block *vsb)
{
    // Declaración de variables (ISO C90)
    struct buffer_head *bh;
    struct assoofs_super_block_info *sb;

    printk(KERN_INFO "assoofs_save_sb_info: request\n");

    // Cargamos la información persistente del superbloque
    sb = vsb->s_fs_info;

    // sb_bread se utiliza aquí para leer el superbloque del dispositivo de bloques (el bloque 0)
    bh = sb_bread(vsb, ASSOOFS_SUPERBLOCK_BLOCK_NUMBER);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_save_sb_info: Reading the superblock failed\n");
        return;
    }

    // Sobreescribimos el contenido del buffer con la información persistente del superbloque
    bh->b_data = (char *)sb;

    // Marcamos el buffer como modificado
    mark_buffer_dirty(bh);

    // Sincrionizamos el buffer con el disco para reflejar los cambios
    sync_dirty_buffer(bh);

    // Liberamos el buffer con brelse
    brelse(bh);
}

int assoofs_sb_get_a_freeblock(struct super_block *sb, uint64_t *block)
{
    // Declaración de variables (ISO C90)
    struct assoofs_super_block_info *assoofs_sb;
    int i;

    printk(KERN_INFO "assoofs_sb_get_a_freeblock: request\n");

    // Asignamos la información persistente del superbloque a una variable
    assoofs_sb = sb->s_fs_info;

    // Recorremos el mapa de bits de bloques libres en busca del primer bloque libre (bit a 1)
    // Empezamos por el bloque 2 porque el bloque 0 es el superbloque y el bloque 1 es el almacén de inodos)
    for (i = 2; i < ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED; i++)
        // Si el bit i-ésimo es 1, hemos encontrado un bloque libre
        if (assoofs_sb->free_blocks & (1 << i))
            break;

    // Comprobamos que no hayamos alcanzado el límite de bloques de objetos soportados por el sistema de ficheros (restamos 2 por el superbloque y el almacén de inodos)
    if (i >= ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED - 2)
    {
        printk(KERN_ERR "assoofs_sb_get_a_freeblock: No more free blocks available");
        return -1;
    }

    // Asignamos el bloque libre encontrado a la variable block
    *block = i;

    // Marcamos el bloque como ocupado (bit a 0)
    assoofs_sb->free_blocks &= ~(1 << i);

    // Guardamos la información persistente del superbloque
    assoofs_save_sb_info(sb);

    // Devolvemos 0 para indicar que todo ha ido bien
    return 0;
}

void assoofs_add_inode_info(struct super_block *sb, struct assoofs_inode_info *inode)
{
    // Declaración de variables (ISO C90)
    struct buffer_head *bh;
    struct assoofs_super_block_info *assoofs_sb;
    struct assoofs_inode_info *inode_info;

    printk(KERN_INFO "assoofs_add_inode_info: request\n");

    // Asignamos la información persistente del superbloque a una variable
    assoofs_sb = sb->s_fs_info;

    // sb_bread se utiliza aquí para leer el bloque que contiene el almacén de inodos (el bloque 1)
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_add_inode_info: Reading the inode store failed\n");
        return;
    }

    // Obtenemos un puntero al primer inodo del almacén de inodos
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info = (struct assoofs_inode_info *)bh->b_data;
    // Movemos el puntero al final del almacén de inodos para añadir el nuevo inodo
    inode_info += assoofs_sb->inodes_count;
    // Copiamos la información persistente del inodo en el almacén de inodos
    memcpy(inode_info, inode, sizeof(struct assoofs_inode_info));

    // Marcamos el buffer como modificado
    mark_buffer_dirty(bh);

    // Sinconizamos el buffer con el disco para reflejar los cambios
    sync_dirty_buffer(bh);

    // Actualizamos el contador de inodos del superbloque
    assoofs_sb->inodes_count++;

    // Guardamos la información persistente del superbloque
    assoofs_save_sb_info(sb);

    // Liberamos el buffer con brelse
    brelse(bh);
}

struct assoofs_inode_info *assoofs_search_inode_info(struct super_block *sb, struct assoofs_inode_info *start, struct assoofs_inode_info *search)
{
    // Declaración de variables (ISO C90)
    uint64_t count = 0;

    printk(KERN_INFO "assoofs_search_inode_info: request\n");

    // Recorremos el almacén de inodos desde el inicio hasta encontrar el inodo buscado o hasta llegar al final
    while (start->inode_no != search->inode_no && count < ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count)
    {
        count++;
        start++;
    }

    // Verificamos que el inodo buscado se haya encontrado
    if (start->inode_no == search->inode_no)
        return start;
    else
        return NULL;
}

int assoofs_save_inode_info(struct super_block *sb, struct assoofs_inode_info *inode_info)
{
    // Declaración de variables (ISO C90)
    struct buffer_head *bh;
    struct assoofs_inode_info *inode_pos;

    printk(KERN_INFO "assoofs_save_inode_info: request\n");

    // sb_bread se utiliza aquí para leer el bloque que contiene el almacén de inodos (el bloque 1)
    bh = sb_bread(sb, ASSOOFS_INODESTORE_BLOCK_NUMBER);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_save_inode_info: Reading the inode store failed\n");
        return -1;
    }

    // Buscamos los datos del inodo en el almacén de inodos
    inode_pos = assoofs_search_inode_info(sb, (struct assoofs_inode_info *)bh->b_data, inode_info);
    if (!inode_pos)
    {
        printk(KERN_ERR "assooofs_save_inode_info: Inode not found\n");
        return -1;
    }

    // Actualizamos el inodo en el almacén de inodos
    memcpy(inode_pos, inode_info, sizeof(*inode_pos));

    // Marcamos el buffer como modificado
    mark_buffer_dirty(bh);

    // Sinconizamos el buffer con el disco para reflejar los cambios
    sync_dirty_buffer(bh);

    // Liberamos el buffer con brelse
    brelse(bh);

    // Devolvemos 0 para indicar que todo ha ido bien
    return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de operaciones sobre ficheros
// +++++++++++++++++++++++++++++++++++++++++++++++++++++

ssize_t assoofs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    // Declaración de variables (ISO C90)
    int nbytes;
    struct super_block *sb;
    struct assoofs_inode_info *inode_info;
    struct buffer_head *bh;
    char *buffer;

    printk(KERN_INFO "assoofs_read: request\n");

    // 1. Obtenemos la información persistente del superbloque
    sb = filp->f_path.dentry->d_inode->i_sb;

    // 2. Obtenemos la información persistente del inodo
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info = filp->f_path.dentry->d_inode->i_private;

    // 3. Comprobamos que no hayamos llegado al final del fichero con el puntero de posición
    if (*ppos >= inode_info->file_size)
        return 0;

    // 4. Accedemos al contenido del fichero y obtenemos un puntero al contenido del fichero
    bh = sb_bread(sb, inode_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_read: Reading the block number [%llu] failed\n", inode_info->data_block_number);
        return -1;
    }
    buffer = (char *)bh->b_data;

    // 5. Copiamos el contenido del fichero al buffer de usuario
    // Incrementamos el buffer para que lea a partir de donde se quedó
    buffer += *ppos;
    // Calculamos el número de bytes que podemos leer a partir de la posición actual (mínimo entre la cantidad restante en el fichero y len)
    nbytes = min((size_t)inode_info->file_size - (size_t)*ppos, len);
    // Copiamos los bytes del fichero al buffer de usuario (argumento buf)
    if (copy_to_user(buf, buffer, nbytes) != 0)
    {
        printk(KERN_ERR "assoofs_read: Error copying file contents to user buffer\n");
        return -1;
    }

    // 7. Actualizamos el puntero de posición
    *ppos += nbytes;

    // 8. Liberamos el buffer con brelse
    brelse(bh);

    // 9. Devolvemos el número de bytes leídos
    return nbytes;
}

ssize_t assoofs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
    // Declaración de variables (ISO C90)
    struct super_block *sb;
    struct assoofs_inode_info *inode_info;
    struct buffer_head *bh;
    char *buffer;

    printk(KERN_INFO "assoofs_write: request\n");

    // 1. Obtenemos la información persistente del superbloque
    sb = filp->f_path.dentry->d_inode->i_sb;

    // 2. Obtenemos la información persistente del inodo
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info = filp->f_path.dentry->d_inode->i_private;

    // 3. Comprobamos que el valor de ppos sumado a al tamaño de los datos a escribir no supere el tamaño máximo de un fichero
    if (*ppos + len > ASSOOFS_DEFAULT_BLOCK_SIZE)
    {
        printk(KERN_ERR "assooofs_write: The file is too large to write it completely\n");
        return -1;
    }

    // 4. Accedemos al contenido del fichero y obtenemos un puntero al contenido del fichero
    bh = sb_bread(sb, inode_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assooofs_write: Reading the block number [%llu] failed\n", inode_info->data_block_number);
        return -1;
    }
    buffer = (char *)bh->b_data;
    // Incrementamos el buffer para que escriba a partir de donde se quedó
    buffer += *ppos;

    // 5. Copiamos el contenido del buffer de usuario al fichero
    if (copy_from_user(buffer, buf, len) != 0)
    {
        printk(KERN_ERR "assooofs_write: Error copying file contents from user buffer\n");
        return -1;
    }

    // 6. Actualizamos el puntero de posición
    *ppos += len;

    // 7. Marcamos el buffer como modificado y sincronizamos el buffer con el disco para reflejar los cambios
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);

    // 8. Actualizamos el tamaño del fichero
    inode_info->file_size = *ppos;
    if (assoofs_save_inode_info(sb, inode_info) != 0)
    {
        printk(KERN_ERR "assooofs_write: Error saving inode info\n");
        return -1;
    }

    // 9. Liberamos el buffer con brelse
    brelse(bh);

    // 10. Devolvemos el número de bytes escritos
    return len;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de operaciones sobre directorios
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static int assoofs_iterate(struct file *filp, struct dir_context *ctx)
{
    // Declaración de variables (ISO C90)
    int i;
    struct inode *inode;
    struct super_block *sb;
    struct assoofs_inode_info *inode_info;
    struct buffer_head *bh;
    struct assoofs_dir_record_entry *record;

    printk(KERN_INFO "assoofs_iterate: request\n");

    // 1. Obtenemos el inodo y el superbloque
    inode = filp->f_path.dentry->d_inode;
    sb = inode->i_sb;
    // Obtenemos la información persistente del inodo
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info = inode->i_private;

    // 2. Comprobamos que el contexto del directorio ya ha sido creado
    // Esto asegura que no se repitan entradas del directorio al llamar a la función repetidas veces (bucle infinito)
    if (ctx->pos != 0)
        return 0;

    // 3. Comprobamos que el inodo es un directorio
    if ((!S_ISDIR(inode_info->mode)))
        return -1;

    // 4. Rellenamos el contexto del directorio con las entradas del directorio
    // Accedemos al bloque de disco con el contenido del directorio
    bh = sb_bread(sb, inode_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_iterate: Reading the block number [%llu] failed\n", inode_info->data_block_number);
        return -1;
    }
    // Declaramos un puntero a la primera entrada del directorio (permite acceder a las demás entradas)
    record = (struct assoofs_dir_record_entry *)bh->b_data;

    // Recorremos las entradas del directorio
    for (i = 0; i < inode_info->dir_children_count; i++)
    {
        // Agregamos la entrada del directorio al contexto del directorio
        dir_emit(ctx, record->filename, ASSOOFS_FILENAME_MAXLEN, record->inode_no, DT_UNKNOWN);

        // Incrementamos la posición con el tamaño de la nueva entrada para que el contexto del directorio apunte a la siguiente entrada
        ctx->pos += sizeof(struct assoofs_dir_record_entry);

        // Incrementamos el puntero a la siguiente entrada
        record++;
    }

    // Liberamos el buffer con brelse
    brelse(bh);

    // Si todo ha ido bien, devolvemos 0
    return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de operaciones sobre inodos
// +++++++++++++++++++++++++++++++++++++++++++++++++++

struct dentry *assoofs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    // Declaración de variables (ISO C90)
    int i;
    struct assoofs_inode_info *parent_info;
    struct super_block *sb;
    struct buffer_head *bh;
    struct assoofs_dir_record_entry *record;
    struct inode *inode;

    printk(KERN_INFO "assoofs_lookup: request\n");

    // 1. Acceder al bloque de disco con el contenido del directorio apuntado por parent_inode
    // Declaramos un puntero con la información persistente del inodo padre
    parent_info = parent_inode->i_private;
    // Preparamos un puntero al superbloque
    sb = parent_inode->i_sb;
    // sb_bread se utiliza aquí para leer el bloque de disco con el contenido del directorio apuntado por parent_inode
    bh = sb_bread(sb, parent_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_lookup: Reading the block number [%llu] failed\n", parent_info->data_block_number);
        return NULL;
    }

    // 2. Recorrer las entradas del directorio en busca de la entrada que coincide con child_dentry
    // Declaramos un puntero a la primera entrada del directorio (permite acceder a las demás entradas)
    record = (struct assoofs_dir_record_entry *)bh->b_data;
    // Recorremos las entradas del directorio en busca de la entrada que coincide con child_dentry
    for (i = 0; i < parent_info->dir_children_count; i++)
    {
        // Comprobamos si el nombre de la entrada coincide con el nombre del dentry hijo
        if (strcmp(record->filename, child_dentry->d_name.name) == 0)
        {
            // Obtenemos el inodo del hijo
            inode = assoofs_get_inode(sb, record->inode_no);
            if (!inode)
            {
                printk(KERN_ERR "assooofs_lookup: inode not found\n");
                return NULL;
            }
            // Inicializamos el inodo, asignando propietario y permisos
            inode_init_owner(sb->s_user_ns, inode, parent_inode, ((struct assoofs_inode_info *)inode->i_private)->mode);
            // Agregamos el directorio hijo al directorio padre
            d_add(child_dentry, inode);

            return NULL;
        }
        record++;
    }

    return NULL;
}

static int assoofs_create(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    // Declaración de variables (ISO C90)
    struct super_block *sb;
    struct buffer_head *bh;
    struct assoofs_inode_info *parent_inode_info;
    struct assoofs_dir_record_entry *dir_contents;
    struct inode *inode;
    struct assoofs_inode_info *inode_info;
    uint64_t count;

    printk(KERN_INFO "assoofs_create: request\n");

    // 1. Creamos el nuevo inodo
    // 1.1. Preparamos un puntero al superbloque
    sb = dir->i_sb;
    count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count;
    // Creamos un nuevo inodo
    inode = new_inode(sb);
    // Asignamos el número de inodo
    inode->i_ino = count + 1;
    // Asignamos el superbloque
    inode->i_sb = sb;
    // Asignamos las operaciones sobre inodos
    inode->i_op = &assoofs_inode_ops;
    // Asignar fecha del sistema a los campos i_atime, i_mtime, i_ctime
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

    // 1.2. Comprobamos si count a superado el número máximo de objetos soportados por el sistema de archivos (restamos 2 por el superbloque y el almacén de inodos)
    if (count >= (ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED - 2))
    {
        printk(KERN_ERR "assoofs_create: Maximum number of objects supported reached\n");
        return -1;
    }

    // 1.3. Guardamos la información persistente del inodo en el campo i_private
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info->inode_no = inode->i_ino;
    inode_info->mode = mode;
    inode_info->file_size = 0;

    inode->i_private = inode_info;

    // 1.4. Asignamos las operaciones sobre ficheros al inodo
    inode->i_fop = &assoofs_file_operations;

    // 1.5. Asignamos el propietario del inodo y los permisos
    inode_init_owner(sb->s_user_ns, inode, dir, mode);

    // 1.6. Agregamos el inodo al árbol de inodos del sistema de archivos
    d_add(dentry, inode);

    // 1.7. Asignamos al inodo un bloque de datos
    if (assoofs_sb_get_a_freeblock(sb, &inode_info->data_block_number) != 0)
    {
        printk(KERN_ERR "assoofs_create: No more free blocks\n");
        return -1;
    }

    // 1.8. Guardamos la información persistente del inodo en el almacén de inodos
    assoofs_add_inode_info(sb, inode_info);

    // 2. Creamos una entrada en el directorio padre para el nuevo inodo
    // 2.1. Leemos el bloque de disco con el contenido del directorio padre
    parent_inode_info = dir->i_private;
    // sb_bread se utiliza aquí para leer el bloque de disco con el contenido del directorio apuntado por parent_inode
    bh = sb_bread(sb, parent_inode_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_create: Reading the block number [%llu] failed\n", parent_inode_info->data_block_number);
        return -1;
    }
    // 2.2. Creamos una nueva entrada en el directorio padre con los datos del nuevo inodo
    dir_contents = (struct assoofs_dir_record_entry *)bh->b_data;
    // Desplazamos el puntero dir_contents hasta la última entrada del directorio
    dir_contents += parent_inode_info->dir_children_count;
    // inode_info es la información persistente del inodo creado en el paso 2.
    dir_contents->inode_no = inode_info->inode_no;

    // Copiamos el nombre del fichero en la entrada del directorio
    strcpy(dir_contents->filename, dentry->d_name.name);
    // mark_buffer_dirty se utiliza para marcar el buffer como modificado
    mark_buffer_dirty(bh);
    // sync_dirty_buffer se utiliza para sincronizar el buffer con el disco (es decir, escribir el buffer en el disco)
    sync_dirty_buffer(bh);
    // Liberamos el buffer
    brelse(bh);

    // 3. Actualizamos la información persistente del directorio padre
    // 3.1. Incrementamos el número de ficheros hijo del directorio padre
    parent_inode_info->dir_children_count++;
    // 3.2. Escribimos en el disco la información persistente del directorio padre
    if (assoofs_save_inode_info(sb, parent_inode_info) != 0)
    {
        printk(KERN_ERR "assoofs_create: Error while saving inode info\n");
        return -1;
    }

    // Si todo ha ido bien, devolvemos 0
    return 0;
}

static int assoofs_mkdir(struct user_namespace *mnt_userns, struct inode *dir, struct dentry *dentry, umode_t mode)
{
    // Declaración de variables (ISO C90)
    struct super_block *sb;
    struct buffer_head *bh;
    struct assoofs_inode_info *parent_inode_info;
    struct assoofs_dir_record_entry *dir_contents;
    struct inode *inode;
    struct assoofs_inode_info *inode_info;
    uint64_t count;

    printk(KERN_INFO "assoofs_mkdir: request\n");

    // 1. Creamos el nuevo inodo
    // 1.1. Preparamos un puntero al superbloque
    sb = dir->i_sb;
    count = ((struct assoofs_super_block_info *)sb->s_fs_info)->inodes_count;
    // Creamos un nuevo inodo
    inode = new_inode(sb);
    // Asignamos el número de inodo
    inode->i_ino = count + 1;
    // Asignamos el superbloque
    inode->i_sb = sb;
    // Asignamos las operaciones sobre inodos
    inode->i_op = &assoofs_inode_ops;
    // Asignar fecha del sistema a los campos i_atime, i_mtime, i_ctime
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

    // 1.2. Comprobamos si count a superado el número máximo de objetos soportados por el sistema de archivos (restamos 2 por el superbloque y el almacén de inodos)
    if (count >= (ASSOOFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED - 2))
    {
        printk(KERN_ERR "assoofs_mkdir: Maximum number of objects supported reached\n");
        return -1;
    }

    // 1.3. Guardamos la información persistente del inodo en el campo i_private
    inode_info = kmem_cache_alloc(assoofs_inode_cache, GFP_KERNEL);
    inode_info->inode_no = inode->i_ino;
    inode_info->mode = S_IFDIR | mode;
    inode_info->file_size = 0;

    inode->i_private = inode_info;

    inode_info->dir_children_count = 0;

    // 1.4. Asignamos las operaciones sobre directorios al inodo
    inode->i_fop = &assoofs_dir_operations;

    // 1.5. Asignamos el propietario del inodo y los permisos
    inode_init_owner(sb->s_user_ns, inode, dir, inode_info->mode);

    // 1.6. Agregamos el inodo al árbol de inodos del sistema de archivos
    d_add(dentry, inode);

    // 1.7. Asignamos al inodo un bloque de datos
    if (assoofs_sb_get_a_freeblock(sb, &inode_info->data_block_number) != 0)
    {
        printk(KERN_ERR "assoofs_mkdir: No more free blocks\n");
        return -1;
    }

    // 1.8. Guardamos la información persistente del inodo en el almacén de inodos
    assoofs_add_inode_info(sb, inode_info);

    // 2. Creamos una entrada en el directorio padre para el nuevo inodo
    // 2.1. Leemos el bloque de disco con el contenido del directorio padre
    parent_inode_info = dir->i_private;
    // sb_bread se utiliza aquí para leer el bloque de disco con el contenido del directorio apuntado por parent_inode
    bh = sb_bread(sb, parent_inode_info->data_block_number);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_mkdir: Reading the block bitmap failed\n");
        return -1;
    }
    // 2.2. Creamos una nueva entrada en el directorio padre con los datos del nuevo inodo
    dir_contents = (struct assoofs_dir_record_entry *)bh->b_data;
    // Desplazamos el puntero dir_contents hasta la última entrada del directorio
    dir_contents += parent_inode_info->dir_children_count;
    // inode_info es la información persistente del inodo creado en el paso 2.
    dir_contents->inode_no = inode_info->inode_no;

    // Copiamos el nombre del fichero en la entrada del directorio
    strcpy(dir_contents->filename, dentry->d_name.name);
    // mark_buffer_dirty se utiliza para marcar el buffer como modificado
    mark_buffer_dirty(bh);
    // sync_dirty_buffer se utiliza para sincronizar el buffer con el disco (es decir, escribir el buffer en el disco)
    sync_dirty_buffer(bh);
    // Liberamos el buffer
    brelse(bh);

    // 3. Actualizamos la información persistente del directorio padre
    // 3.1. Incrementamos el número de ficheros hijo del directorio padre
    parent_inode_info->dir_children_count++;
    // 3.2. Escribimos en el disco la información persistente del directorio padre
    if (assoofs_save_inode_info(sb, parent_inode_info) != 0)
    {
        printk(KERN_ERR "assoofs_mkdir: Error while saving inode info\n");
        return -1;
    }

    // Si todo ha ido bien, devolvemos 0
    return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de operaciones de superbloque
// +++++++++++++++++++++++++++++++++++++++++++++++++++++

int assoofs_destroy_inode(struct inode *inode)
{
    struct assoofs_inode *inode_info = inode->i_private;

    printk(KERN_INFO "Freeing private data of inode %p ( %lu)\n", inode_info, inode->i_ino);

    kmem_cache_free(assoofs_inode_cache, inode_info);

    return 0;
}

int assoofs_fill_super(struct super_block *sb, void *data, int silent)
{
    // Declaración de variables (ISO C90)
    struct buffer_head *bh;
    struct assoofs_super_block_info *assoofs_sb;
    struct inode *root_inode;

    printk(KERN_INFO "assoofs_fill_super request\n");
    // 1.- Leer la información persistente del superbloque del dispositivo de bloques
    // sb_bread se utiliza aquí para leer el superbloque del dispositivo de bloques (el bloque 0)
    bh = sb_bread(sb, ASSOOFS_SUPERBLOCK_BLOCK_NUMBER);
    if (!bh)
    {
        printk(KERN_ERR "assoofs_fill_super: unable to read superblock\n");
        return -1;
    }
    // Para acceder a los campos del superbloque, primero hay que asignar bh->b_data a un puntero de tipo assoofs_super_block_info
    assoofs_sb = (struct assoofs_super_block_info *)bh->b_data;
    // Ya podemos liberar el buffer_head con brelse
    brelse(bh);

    // 2.- Comprobar los parámetros del superbloque
    // Esto es necesario para comprobar que el superbloque que se ha leído es realmente un superbloque de assoofs
    // 2.1.- Comprobar el número mágico
    if (assoofs_sb->magic != ASSOOFS_MAGIC)
    {
        printk(KERN_ERR "assoofs_fill_super: wrong magic number (0x%llx)\n", assoofs_sb->magic);
        return -1;
    }
    // 2.2.- Comprobar el tamaño del bloque
    if (assoofs_sb->block_size != ASSOOFS_DEFAULT_BLOCK_SIZE)
    {
        printk(KERN_ERR "assoofs_fill_super: wrong block size (%llu)\n", assoofs_sb->block_size);
        return -1;
    }

    // 3.- Escribir la información persistente leída del dispositivo de bloques en el superbloque sb
    // El campo s_magic es el número mágico que identifica el sistema de ficheros
    sb->s_magic = ASSOOFS_MAGIC;
    // El campo s_maxbytes es el tamaño máximo de fichero
    sb->s_maxbytes = ASSOOFS_DEFAULT_BLOCK_SIZE;
    // El campo s_op define las operaciones que se pueden realizar en el sistema de ficheros
    sb->s_op = &assoofs_sops;
    // El campo s_fs_info es un puntero a una estructura que contiene información persistente del superbloque
    // Esto evita tener que acceder continuamente al bloque 0 (menos lecturas)
    sb->s_fs_info = assoofs_sb;

    // 4.- Crear el inodo raíz y asignarle operaciones sobre inodos (i_op) y sobre directorios (i_fop)
    // 4.1.- Creamos el inodo raíz
    root_inode = new_inode(sb);

    // 4.2.- Inicializamos el inodo raíz, asignando propietario y permisos
    // El inodo no tiene padre, por lo que se le pasa NULL
    // El tipo de fichero es un directorio, por lo que se le pasa S_IFDIR (S_IFREG para ficheros)
    inode_init_owner(sb->s_user_ns, root_inode, NULL, S_IFDIR);

    // 4.3.- Asignamos la información del inodo raíz
    // El número de inodo es ASSOOFS_ROOTDIR_INODE_NUMBER (1)
    root_inode->i_ino = ASSOOFS_ROOTDIR_INODE_NUMBER;
    // Definimos el puntero al superbloque
    root_inode->i_sb = sb;
    // Asignamos las operaciones de inodo
    root_inode->i_op = &assoofs_inode_ops;
    // Asignamos las operaciones de directorio
    root_inode->i_fop = &assoofs_dir_operations;
    // Establecemos las fechas de acceso, modificación y cambio del inodo al tiempo actual
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode);
    // Almacena la información persistente del inodo raíz
    root_inode->i_private = assoofs_get_inode_info(sb, ASSOOFS_ROOTDIR_INODE_NUMBER);

    // 5. - Guardar el inodo raíz en el superbloque y marcarlo como raíz
    // Se marca como tal y se guarda en el campo s_root del superbloque
    sb->s_root = d_make_root(root_inode);

    return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de operaciones de montaje
// +++++++++++++++++++++++++++++++++++++++++++++++++

static struct dentry *assoofs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    struct dentry *ret;
    printk(KERN_INFO "assoofs_mount request\n");

    // Montar el dispositivo de bloque con mount_bdev
    // assofs_fill_super es la función que se ejecutará para rellenar el superbloque
    ret = mount_bdev(fs_type, flags, dev_name, data, assoofs_fill_super);

    // Control de errores a partir del valor de ret. En este caso se puede utilizar la macro IS_ERR: if (IS_ERR(ret)) ...
    if (IS_ERR(ret))
    {
        printk(KERN_ERR "assoofs_mount: error mounting fs\n");
    }
    return ret;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Definición de funciones de inicialización y descarga del módulo
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static int __init assoofs_init(void)
{
    // Declaración de variables (ISO C90)
    int ret;

    printk(KERN_INFO "assoofs_init: request\n");

    // Registrar el sistema de archivos en el kernel
    ret = register_filesystem(&assoofs_type);

    // Control de errores a partir del valor de ret
    if (ret != 0)
    {
        printk(KERN_ERR "assoofs_init: can't register filesystem\n");
        return ret;
    }

    // Inicializamos la caché de inodos
    assoofs_inode_cache = kmem_cache_create("assoofs_inode_cache", sizeof(struct assoofs_inode_info), 0, (SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD), NULL);

    return ret;
}

static void __exit assoofs_exit(void)
{
    // Declaración de variables (ISO C90)
    int ret;

    printk(KERN_INFO "assoofs_exit: request\n");

    // Eliminamos el sistema de archivos del kernel
    ret = unregister_filesystem(&assoofs_type);

    // Control de errores a partir del valor de ret
    if (ret != 0)
    {
        printk(KERN_ERR "assoofs_exit: can't unregister filesystem\n");
    }

    // Destruimos la caché de inodos
    kmem_cache_destroy(assoofs_inode_cache);
}

module_init(assoofs_init);
module_exit(assoofs_exit);
