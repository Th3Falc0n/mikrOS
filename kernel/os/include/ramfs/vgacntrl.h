#ifndef VGACNTRL_H_
#define VGACNTRL_H_

#include "vfs.h"

#define CMD_SET    0
#define CMD_CLEAR  1
#define CMD_SCROLL 2

struct vga_command {
    uint16_t command;
    uint16_t offset;
    uint16_t value;
};

struct kfs_driver* ramfs_vga_driver_struct();
uint32_t           ramfs_vga_writers();
uint32_t           ramfs_vga_available(struct res_handle* handle);
struct res_kfile*  ramfs_vga_create(uint32_t* args);
uint32_t           ramfs_vga_read(struct res_handle* handle, void* dest, uint32_t length);
uint32_t           ramfs_vga_write(struct res_handle* handle, void* src, uint32_t length);
struct res_handle* ramfs_vga_open(struct res_kfile* kf, uint32_t filemode);
uint32_t           ramfs_vga_close(struct res_handle* handle);

#endif /* VGACNTRL_H_ */
