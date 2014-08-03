#ifndef FIFO_H_
#define FIFO_H_

#include "vfs.h"

struct fifo_buffer {
    void* buffer;
    uint32_t size;
    uint32_t pos;

    uint32_t readers;
    uint32_t writers;
};

void               ramfs_fifo_init();
struct kfs_driver* ramfs_fifo_driver_struct();

struct res_kfile*  ramfs_fifo_create(uint32_t* args);
uint32_t           ramfs_fifo_read  (struct res_handle* handle, void* dest, uint32_t pos, uint32_t length);
uint32_t           ramfs_fifo_write (struct res_handle* handle, void* src,  uint32_t pos, uint32_t length);
struct res_handle* ramfs_fifo_open  (struct res_kfile* kf, uint32_t filemode);
uint32_t           ramfs_fifo_close (struct res_handle* handle);

#endif /* FIFO_H_ */
