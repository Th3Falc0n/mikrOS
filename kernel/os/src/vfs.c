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

void vfs_exec(char* ip, char* args[], struct task* task) {
    char* path = strclone(ip);
    if(vfs_exists(path)) {
        uint32_t elf_mod_pdir;

        if(task == 0) {
            elf_mod_pdir = vmm_create_pagedir();
            task = init_task(elf_mod_pdir, (void*)1);
            kprintf("[exec] Initialized new task...\n");
        }
        else
        {
            elf_mod_pdir = task->phys_pdir;
            kprintf("[exec] Replacing old task...\n");
        }

        struct res_handle* handle = vfs_open(path, FM_EXEC | FM_READ);

        if(handle) {
            uint32_t size = vfs_available(handle);
            if(size == 0) {
                kprintf("[exec] %s is empty\n", path);
                return;
            }

            void* modsrc = malloc(size);

            uint32_t res = vfs_read(handle, modsrc, size, 1);

            if(res != RW_OK) {
                kprintf("[exec] Error while reading %s\n");
                free(modsrc);
                return;
            }

            uint32_t old_pdir = vmm_get_current_pagedir();

            //**********************************************************************************************************
            if(task != get_current_task())  vmm_activate_pagedir(elf_mod_pdir);

            struct elf_header* header = modsrc;
            struct elf_program_header* ph;

            /* Ist es ueberhaupt eine ELF-Datei? */
            if (header->magic != ELF_MAGIC) {
                kprintf("[exec] Invalid ELF-Magic in %s!\n", path);
                free(modsrc);
                return;
            }

            void* elf_mod_entry = (void*) (header->entry);

            ph = (struct elf_program_header*) (((char*) header) + header->ph_offset);

            for (uint32_t n = 0; n < header->ph_entry_count; n++, ph++) {
                void* dest = (void*) ph->virt_addr;
                void* src = ((char*) header) + ph->offset;

                /* Nur Program Header vom Typ LOAD laden */
                if (ph->type != 1) {
                    continue;
                }

                for (uint32_t offset = 0; offset < ph->mem_size; offset += 0x1000) {
                    vmm_free(dest + offset);
                    vmm_alloc_addr(dest + offset, 0);
                }

                memcpy(dest, src, ph->file_size);
            }

            task->args = args;
            task->cpuState->eip = (uint32_t) elf_mod_entry;

            if(task != get_current_task()) vmm_activate_pagedir(old_pdir);

            kprintf("[exec] Executed %s\n", path);
            free(modsrc);
        }
        else
        {
            kprintf("[exec] %s doesn't exist\n", path);
        }
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
