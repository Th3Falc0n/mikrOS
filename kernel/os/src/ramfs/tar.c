#include "ramfs/tar.h"
#include "ramfs/block.h"
#include "vfs.h"

static uint32_t tar_parse_number(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

void tar_load_ramfs(void* tarball) {
    void* address = tarball;

    uint32_t i;

    kprintf("[initrfs] Starting unpacking tarball at %x\n", address);

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header*)address;

        if (header->name[0] == '\0')
            break;

        uint32_t size = tar_parse_number(header->size);
        address += 512;

        if(size != 0) {
            kprintf("[initrfs] Extracting %s (%d bytes)\n", header->name, size);

            vfs_create_kfile(header->name, ramfs_block_driver_struct(), &size);
            struct res_handle* h = vfs_open(header->name, FM_WRITE);

            uint32_t err = vfs_write(h, address, size, 1);
            if(err) {
                kprintf("[initrfs] VFS ERROR: %d\n", err);
            }

            vfs_close(h);
        }
        else
        {
            kprintf("[initrfs] Creating dir %s (%d bytes)\n", header->name, size);
            vfs_create_dir(header->name);
        }

        address += (size / 512) * 512;

        if (size % 512)
            address += 512;

    }

    kprintf("[initrfs] Finished unpacking tarball\n");
}
