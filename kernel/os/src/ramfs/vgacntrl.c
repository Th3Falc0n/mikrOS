#include "ramfs/vgacntrl.h"

uint16_t* vga_buffer = (void*) 0xB8000;
uint32_t  vga_writers = 0;

struct kfs_driver vga_driver = {
    .rread     = ramfs_vga_read,
    .rwrite    = ramfs_vga_write,
    .open      = ramfs_vga_open,
    .create    = ramfs_vga_create,
    .close     = ramfs_vga_close,
    .available = ramfs_vga_available,
    .drvname   = "VGA_CONTROL"
};

struct kfs_driver* ramfs_vga_driver_struct() {
    return &vga_driver;
}

uint32_t ramfs_vga_writers() {
    return vga_writers;
}

uint32_t ramfs_vga_available(struct res_handle* handle) {
    return sizeof(struct vga_command);
}

struct res_kfile* ramfs_vga_create(uint32_t* args) {
    return malloc(sizeof(struct res_kfile));
}

uint32_t ramfs_vga_read(struct res_handle* handle, void* dest, uint32_t length) {
    return RW_ERR_DRIVER; //Deadlock if lib-mikrOS tries to block until successfull read when returning 0 so we return error code 1 (length+1)
}

uint32_t ramfs_vga_write(struct res_handle* handle, void* src, uint32_t length) {
    if(length != sizeof(struct vga_command)) return RW_ERR_DRIVER;

    struct vga_command* vgac = src;

    if(vgac->command == CMD_SET) {
        if(vgac->offset < (25 * 80)) vga_buffer[vgac->offset] = vgac->value;
    }

    if(vgac->command == CMD_CLEAR) {
        for(int i = 0; i < (80 * 25); i++) {
            vga_buffer[i] = 0;
        }
    }

    if(vgac->command == CMD_SCROLL) {
        int i;
        for (i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }

        for (; i < 25 * 80; i++) {
            vga_buffer[i] = 0;
        }
    }

    return RW_OK; //Deadlock if lib-mikrOS tries to block until successfull write when returning 0 so we return error code 1 (length+1)
}

struct res_handle* ramfs_vga_open(struct res_kfile* kf, uint32_t filemode) {
    if(filemode & FM_EXEC) return 0;
    if(filemode & FM_READ) return 0;

    if((filemode & FM_WRITE) && vga_writers != 0) return 0;
    if((filemode & FM_WRITE)) vga_writers++;

    struct res_handle* rethandle = malloc(sizeof(struct res_handle));

    rethandle->filemode = filemode;
    rethandle->position = 0;
    rethandle->res_type = RES_KERNDRV;
    rethandle->res_ptr  = kf;

    return rethandle;
}

uint32_t ramfs_vga_close(struct res_handle* handle) {
    if(handle->filemode & FM_WRITE) vga_writers--;

    free(handle);

    return 0;
}
