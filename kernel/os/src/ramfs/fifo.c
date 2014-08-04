#include "ramfs/fifo.h"

struct fifo_buffer* buffer[65536];

void ramfs_fifo_init() {
    memset(buffer, 0, sizeof(buffer));
}

struct kfs_driver fifo_driver = {
    .rread     = ramfs_fifo_read,
    .rwrite    = ramfs_fifo_write,
    .open      = ramfs_fifo_open,
    .create    = ramfs_fifo_create,
    .close     = ramfs_fifo_close,
    .available = ramfs_fifo_available,
    .drvname   = "FIFO"
};

struct kfs_driver* ramfs_fifo_driver_struct() {
    return &fifo_driver;
}

uint32_t ramfs_fifo_available(struct res_handle* handle) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        return buffer[kf->id]->pos;
    }

    return 0;
}

struct res_kfile* ramfs_fifo_create(uint32_t* args) {
    struct res_kfile* kfret = malloc(sizeof(struct res_kfile));

    for(int i = 0; i < 65536; i++) {
        if(buffer[i] == 0) {
            buffer[i] = malloc(sizeof(struct fifo_buffer));
            buffer[i]->buffer = malloc(args[0]);
            buffer[i]->size   = args[0];

            buffer[i]->pos = 0;
            buffer[i]->writers = 0;
            buffer[i]->readers = 0;

            kfret->id = i;

            return kfret;
        }
    }

    return 0;
}

uint32_t ramfs_fifo_read(struct res_handle* handle, void* dest, uint32_t length) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(length > buffer[kf->id]->size) return RW_ERR_DRIVER; //Error code 2 -> Can't read cause buffer too small
        if(length > buffer[kf->id]->pos)  return RW_BLOCK; //Block until write is possible TODO maybe check if FIFO has writers

        memcpy(dest, buffer[kf->id]->buffer, length);
        memcpy(buffer[kf->id]->buffer, buffer[kf->id]->buffer + length, buffer[kf->id]->pos - length);
        buffer[kf->id]->pos -= length;

        return RW_OK;
    }

    return RW_ERR_DRIVER; //Deadlock if lib-mikrOS tries to block until successfull read when returning 0 so we return error code 1 (length+1)
}

uint32_t ramfs_fifo_write(struct res_handle* handle, void* src, uint32_t length) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(length > buffer[kf->id]->size) return RW_ERR_DRIVER; //Error code 2 -> Can't write cause buffer too small
        if(length > (buffer[kf->id]->size - buffer[kf->id]->pos)) return RW_BLOCK; //Block until write is possible TODO maybe check if FIFO has readers

        memcpy(buffer[kf->id]->buffer + buffer[kf->id]->pos, src, length);
        buffer[kf->id]->pos += length;

        return RW_OK;
    }

    return RW_ERR_DRIVER; //Deadlock if lib-mikrOS tries to block until successfull write when returning 0 so we return error code 1 (length+1)
}

struct res_handle* ramfs_fifo_open(struct res_kfile* kf, uint32_t filemode) {
    if(buffer[kf->id] != 0) {
        if(filemode & FM_EXEC) return 0;
        if((filemode & FM_READ) && buffer[kf->id]->readers > 0) return 0;

        if((filemode & FM_READ)) buffer[kf->id]->readers++;
        if((filemode & FM_WRITE)) buffer[kf->id]->writers++;

        struct res_handle* rethandle = malloc(sizeof(struct res_handle));

        rethandle->filemode = filemode;
        rethandle->position = 0;
        rethandle->res_type = RES_KERNDRV;
        rethandle->res_ptr  = kf;

        return rethandle;
    }

    return 0;
}

uint32_t ramfs_fifo_close(struct res_handle* handle) {
    struct res_kfile* kf = ((struct res_kfile*)handle->res_ptr);

    if(buffer[kf->id] != 0) {
        if(handle->filemode & FM_READ)  buffer[kf->id]->readers--;
        if(handle->filemode & FM_WRITE) buffer[kf->id]->writers--;
    }

    free(handle);

    return 0;
}
