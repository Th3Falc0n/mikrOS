#ifndef VFS_H
#define VFS_H

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "catofdeath.h"

#define RES_SUBDIR  0
#define RES_KERNDRV 1
#define RES_USERDRV 2

#define FM_READ   (1 << 0)
#define FM_WRITE  (1 << 1)
#define FM_EXEC   (1 << 2)
#define FM_CREATE (1 << 3)

#define RW_OK         0
#define RW_BLOCK      1
#define RW_ERR_VFS    2
#define RW_ERR_DRIVER 3

#define SEEK_SET  0
#define SEEK_CUR  1

struct res_handle {
    uint32_t res_type;
    void* res_ptr;

    uint32_t position;
    uint32_t filemode;
};

struct res_node {
    char name[256];

    uint32_t res_type;
    void* res_ptr;

    struct res_node* next;
    struct res_node* parent;
};

struct res_kfile {
    uint32_t id;
    struct kfs_driver* driver;
};


struct kfs_driver {
    uint32_t           (*rread)     (struct res_handle*, void*, uint32_t); //(handle, dest, length)
    uint32_t           (*rwrite)    (struct res_handle*, void*, uint32_t); //(handle, src , length)
    struct res_handle* (*open)      (struct res_kfile*, uint32_t); //(file, filemode)
    uint32_t           (*close)     (struct res_handle*); //(handle)
    uint32_t           (*available) (struct res_handle*); //(handle)

    struct res_kfile*  (*create)    (uint32_t*); //(size)
    char*              drvname;
};

void               vfs_init_root   (void);
void               vfs_debug_ls    (char* path);
uint32_t           vfs_create_dir  (char* path);
uint32_t           vfs_create_kfile(char* path, struct kfs_driver* driver, uint32_t* params);
uint32_t           vfs_remove      (char* path);

struct res_handle* vfs_open        (char* path, uint32_t filemode);
uint32_t           vfs_close       (struct res_handle* handle);
uint32_t           vfs_read        (struct res_handle* handle, void* dest, uint32_t size, uint32_t count);
uint32_t           vfs_write       (struct res_handle* handle, void* src,  uint32_t size, uint32_t count);
uint32_t           vfs_available   (struct res_handle* handle);
uint32_t           vfs_exists      (char* path);
void               vfs_seek        (struct res_handle* handle, uint32_t offset, uint32_t origin);

void               vfs_exec        (char* path, char* args[], struct task* task);

#endif
