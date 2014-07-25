#ifndef VFS_H
#define VFS_H

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#define RES_NLIST 0
#define RES_MOUNT 1
#define RES_MEM   2

struct rs_node {
    char name[256];

    uint32_t res_type;
    void* res_ptr; /*
     * RES_NODE  -> rs_node
     * RES_MOUNT -> rs_drv
     * RES_MEM   -> rs_mem
     */

    struct rs_node* next;
    struct rs_node* parent;
};

struct rs_mfile {
    uint32_t size;
    void* data;
};

struct rs_drv {
    uint32_t pid; //TODO whatever
};

void  vfs_init_root(void);
void  vfs_debug_ls(char* path);
int   vfs_create_path(char* path, uint32_t flags);
int   vfs_create_memfile(char* path, uint32_t flags, void* content, uint32_t size);
int   vfs_remove(char* path);
char* vfs_list_dir(char* path);
int   vfs_remove(char* path);
int   vfs_mount(char* path, struct rs_drv* driver);
int   vfs_umount(char* path);

#endif
