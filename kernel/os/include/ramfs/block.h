#ifndef BLOCK_H_
#define BLOCK_H_

#include "vfs.h"

struct block_buffer {
    void* buffer;
    uint32_t size;

    uint32_t readers;
    uint32_t writers;
};

void               ramfs_block_init();
struct kfs_driver* ramfs_block_driver_struct();

struct res_kfile*  ramfs_block_create    (uint32_t* args);
uint32_t           ramfs_block_read      (struct res_handle* handle, void* dest, uint32_t length);
uint32_t           ramfs_block_write     (struct res_handle* handle, void* src,  uint32_t length);
struct res_handle* ramfs_block_open      (struct res_kfile* kf, uint32_t filemode);
uint32_t           ramfs_block_close     (struct res_handle* handle);
uint32_t           ramfs_block_available (struct res_handle* handle);

#endif /* BLOCK_H_ */
