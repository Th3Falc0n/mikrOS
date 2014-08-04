#ifndef TAR_H_
#define TAR_H_

struct tar_header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

void tar_load_ramfs(void* tarball);

#endif /* TAR_H_ */
