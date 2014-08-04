#include "vfs.h"
#include "ramfs/fifo.h"

struct res_node* root = 0;
struct res_node* current = 0;
struct res_node* temp = 0;

static struct res_node* vfs_find_node(struct res_node* parent, char* name) {
    if(parent->res_type == RES_SUBDIR) {
        struct res_node* ptr = parent->res_ptr;

        while(ptr != 0) {
            if(strcmp(name, ptr->name) == 0) {
                return ptr;
            }

            ptr = ptr->next;
        }
    }

    return 0;
}

static int vfs_insert_node(struct res_node* parent, struct res_node* child) {
    if(parent->res_ptr == 0) {
        parent->res_type = RES_SUBDIR;
    }

    if(parent->res_type == RES_SUBDIR) {
        child->next = parent->res_ptr;
        child->parent = parent;
        parent->res_ptr = child;

        return 0;
    }

    return 1;
}

static int vfs_create_path(char* path) {
    char* sub;
    struct res_node* parent = root;
    struct res_node* child = 0;

    int created = 0;

    sub = strtok(path, "/");
    if(sub[0] == 0) sub = strtok(0, "/");

    while (sub != NULL)
    {
        child = vfs_find_node(parent, sub);

        if(child == 0) {
            child = malloc(sizeof(struct res_node));

            strcpy(child->name, sub);
            child->res_type = RES_SUBDIR;
            child->res_ptr = 0;

            created++;

            if(vfs_insert_node(parent, child)) {
                show_cod(malloc(sizeof(struct cpu_state)), "Tried to insert VFS node in non RES_SUBDIR parent");
            }
        }

        parent = child;

        sub = strtok(0, "/");
    }

    return created;
}

static struct res_node* vfs_get_node(char* path) {
    char* sub;
    struct res_node* parent = root;
    struct res_node* child = 0;

    sub = strtok(path, "/");
    if(sub[0] == 0) sub = strtok(0, "/");

    while (sub != NULL)
    {
        child = vfs_find_node(parent, sub);
        parent = child;
        sub = strtok(0, "/");
    }

    return child;
}

uint32_t vfs_create_dir(char* path) {
    return vfs_create_path(path);
}

uint32_t vfs_create_kfile(char* path, struct kfs_driver* driver, uint32_t* params) {
    char* pathc = malloc(strlen(path) + 1);
    strcpy(pathc, path);

    uint32_t ret = vfs_create_path(pathc);

    static struct res_node* node;
    node = vfs_get_node(path);

    node->res_type = RES_KERNDRV;
    node->res_ptr = driver->create(params);
    ((struct res_kfile*) node->res_ptr)->driver = driver;

    return ret;
}

struct res_handle* vfs_open(char* path, uint32_t filemode) {
    static struct res_node* node;

    node = vfs_get_node(path);

    if(node->res_type == RES_KERNDRV) {
        struct res_kfile* kf = (struct res_kfile*)node->res_ptr;
        struct res_handle* handle = kf->driver->open(kf, filemode);

        return handle;
    }

    return 0;
}

uint32_t vfs_close(struct res_handle* handle) {
    if(handle->res_type == RES_KERNDRV) {
       struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

       return kf->driver->close(handle);
    }

    return 0;
}

uint32_t vfs_read(struct res_handle* handle, void* dest, uint32_t size, uint32_t count) {
    if(handle == 0) return RW_ERR_VFS;

    if(handle->res_type == RES_KERNDRV) {
       struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

       uint32_t res = kf->driver->rread(handle, dest, size * count); //TODO it's not that easy... it shouldn't be size * count but instead count times size (for loop)

       if(res == RW_OK) {
           handle->position += size * count;
           return RW_OK;
       }

       if(res == RW_BLOCK) {
           return RW_BLOCK;
       }

       return RW_ERR_DRIVER;
    }

    return RW_ERR_VFS;
}

uint32_t vfs_write(struct res_handle* handle, void* src,  uint32_t size, uint32_t count) {
    if(handle == 0) return RW_ERR_VFS;

    if(handle->res_type == RES_KERNDRV) {
       struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

       uint32_t res = kf->driver->rwrite(handle, src, size * count); //TODO it's not that easy... it shouldn't be size * count but instead count times size (for loop)

       if(res == RW_OK) {
           handle->position += size * count;
           return RW_OK;
       }

       if(res == RW_BLOCK) {
           return RW_BLOCK;
       }

       return RW_ERR_DRIVER;
    }

    return RW_ERR_VFS;
}

uint32_t vfs_available(struct res_handle* handle) {
    if(handle == 0) return 0;

    if(handle->res_type == RES_KERNDRV) {
        struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

        return kf->driver->available(handle);
    }

    return 0;
}

uint32_t vfs_exists(char* path) {
    if(vfs_get_node(path) != 0) return 1;
    return 0;
}

void vfs_seek(struct res_handle* handle, uint32_t offset, uint32_t origin) {
    if(origin == SEEK_SET) {
        handle->position = offset;
    }

    if(origin == SEEK_CUR) {
        handle->position += offset;
    }
}

void vfs_init_root() {
    if(root == 0) {
        root = malloc(sizeof(struct res_node));
        root->next = 0;
        root->parent = 0;
        strcpy(root->name, "$");
        root->res_ptr = 0;
        root->res_type = RES_SUBDIR;
    }
}

void vfs_debug_ls(char* path) {
    static struct res_node* node;
    node = vfs_get_node(path);
    node = node->res_ptr;

    while(node != 0) {
        char* type = "N/A";

        if(node->res_type == RES_SUBDIR) {
            type = "DIR";
        }

        if(node->res_type == RES_KERNDRV) {
            type = ((struct res_kfile*)node->res_ptr)->driver->drvname;
        }

        kprintf("LS %s: %s [%s]\n", path, node->name, type);

        node = node->next;
    }
}
