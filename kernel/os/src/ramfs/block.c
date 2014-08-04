#include "ramfs/block.h"

struct block_buffer* buffer[65536];

void ramfs_block_init() {
    memset(buffer, 0, sizeof(buffer));
}

struct kfs_driver block_driver = {
    .rread     = ramfs_block_read,
    .rwrite    = ramfs_block_write,
    .open      = ramfs_block_open,
    .create    = ramfs_block_create,
    .close     = ramfs_block_close,
    .available = ramfs_block_available,
    .drvname   = "RAMFILE"
};

struct kfs_driver* ramfs_block_driver_struct() {
    return &block_driver;
}

uint32_t ramfs_block_available(struct res_handle* handle) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        return buffer[kf->id]->size - handle->position;
    }

    return 0;
}

struct res_kfile* ramfs_block_create(uint32_t* args) {
    struct res_kfile* kfret = malloc(sizeof(struct res_kfile));

    for(int i = 0; i < 65536; i++) {
        if(buffer[i] == 0) {
            buffer[i] = malloc(sizeof(struct block_buffer));
            buffer[i]->buffer = malloc(args[0]);
            buffer[i]->size   = args[0];

            buffer[i]->writers = 0;
            buffer[i]->readers = 0;

            kfret->id = i;

            return kfret;
        }
    }

    return 0;
}

uint32_t ramfs_block_read(struct res_handle* handle, void* dest, uint32_t length) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(length > (buffer[kf->id]->size - handle->position)) return RW_ERR_DRIVER; //Can't read that much biatch

        memcpy(dest, buffer[kf->id]->buffer + handle->position, length);
        handle->position += length;

        return RW_OK;
    }

    return RW_ERR_DRIVER; //Deadlock if lib-mikrOS tries to block until successfull read when returning 0 so we return error code 1 (length+1)
}

uint32_t ramfs_block_write(struct res_handle* handle, void* src, uint32_t length) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(length > (buffer[kf->id]->size - handle->position)) return RW_ERR_DRIVER; //Can't write  that much biatch TODO realloc so file may grow

        memcpy(buffer[kf->id]->buffer + handle->position, src, length);
        handle->position += length;

        return RW_OK;
    }

    return RW_ERR_DRIVER; //Deadlock if lib-mikrOS tries to block until successfull write when returning 0 so we return error code 1 (length+1)
}

struct res_handle* ramfs_block_open(struct res_kfile* kf, uint32_t filemode) {
    if(buffer[kf->id] != 0) {
        if((filemode & FM_READ)) buffer[kf->id]->readers++;
        if((filemode & FM_WRITE)) buffer[kf->id]->writers++;

        struct res_handle* rethandle = malloc(sizeof(struct res_handle));

        rethandle->filemode = filemode;
        rethandle->position = 0;
        rethandle->res_type = RES_KERNDRV;
        rethandle->res_ptr  = kf;

        return rethandle;
    }

    return (void*)1;
}

uint32_t ramfs_block_close(struct res_handle* handle) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(handle->filemode & FM_READ)  buffer[kf->id]->readers--;
        if(handle->filemode & FM_WRITE) buffer[kf->id]->writers--;
    }

    free(handle);

    return 0;
}
