#include "vfs.h"
#include "ramfs/fifo.h"

struct res_node* root = 0;
struct res_node* current = 0;
struct res_node* temp = 0;

static void vfs_set_error(uint32_t code) {
    if(get_current_task()) {
        if(get_current_task()->vfserr == PE_NO_ERROR) {
            get_current_task()->vfserr = code;
        }
    }
}

void vfs_reset_error() {
    if(get_current_task()) {
        get_current_task()->vfserr = PE_NO_ERROR;
    }
}

char capfn_buf[513];

static char* vfs_construct_absolute_path_for_node(struct res_node* node) {
    capfn_buf[512] = '\0';
    char* out =  &(capfn_buf[511]);

    *out = '/';

    while(node != root) {
        out -= strlen(node->name);
        strcpy(out, node->name);
        *--out = '/';

        node = node->parent;
    }

    return strclone(out);
}

static struct res_node* vfs_find_node(struct res_node* parent, char* name) {
    if(name == 0) return parent;
    if(name[0] == '\0') return parent;

    if(!strcmp(name, ".")) return parent;
    if(!strcmp(name, "..")) {
        if(parent == root) return parent;
        return parent->parent;
    }

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


static struct res_node* vfs_get_relative_node(struct res_node* parent, char* path) {
    char* sub;
    struct res_node* child = 0;

    if(path == 0) {
        vfs_set_error(PE_INVALID);
        return 0;
    }

    if(path[0] == 0) {
        vfs_set_error(PE_INVALID);
        return 0;
    }

    sub = strtok(path, "/");

    if(sub == NULL) {
        return parent;
    }

    if(sub[0] == 0) sub = strtok(0, "/");

    if(sub == NULL) {
        return parent;
    }

    while (sub != NULL)
    {
        if(parent == 0) return 0;

        child = vfs_find_node(parent, sub);
        parent = child;

        sub = strtok(0, "/");
    }

    return child;
}

static struct res_node* vfs_get_current_task_root_node_for_path(char* path) {
    if(path == 0 || path[0] == '\0') {
        vfs_set_error(PE_INVALID);
        return 0;
    }

    if(path[0] == '/') return root;
    if(get_current_task() == 0) return root;
    if(get_current_task()->execPath == 0) return root;

    return vfs_get_relative_node(root, get_current_task()->execPath);

}

static struct res_node* vfs_get_node(char* path) {
    return vfs_get_relative_node(vfs_get_current_task_root_node_for_path(path), path);
}

static int vfs_create_path(char* path) {
    char* sub;
    struct res_node* parent = vfs_get_current_task_root_node_for_path(path);
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

    if(!(handle->filemode & FM_READ)) return RW_NOFM_READ;

    if(handle->res_type == RES_KERNDRV) {
       struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

       uint32_t res = kf->driver->rread(handle, dest, size * count); //TODO it's not that easy... it shouldn't be size * count but instead count times size (for loop)

       if(res == RW_OK) {
           handle->position += size * count;
           return RW_OK;
       }

       return res;
    }

    return RW_ERR_VFS;
}

uint32_t vfs_write(struct res_handle* handle, void* src,  uint32_t size, uint32_t count) {
    if(handle == 0) return RW_ERR_VFS;

    if(!(handle->filemode & FM_WRITE)) return RW_NOFM_WRITE;

    if(handle->res_type == RES_KERNDRV) {
       struct res_kfile* kf = (struct res_kfile*)handle->res_ptr;

       uint32_t res = kf->driver->rwrite(handle, src, size * count); //TODO it's not that easy... it shouldn't be size * count but instead count times size (for loop)

       if(res == RW_OK) {
           handle->position += size * count;
           return RW_OK;
       }

       return res;
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

uint32_t vfs_exec(char* path, char* args[], char* execPath, char* stdin, char* stdout, char* stderr) {
    path = strclone(path);
    if(!vfs_exists(path)) {
        free(path);
        vfs_set_error(PE_FILE_NOT_FOUND);
        return 0;
    }

    uint32_t elf_mod_pdir;

    elf_mod_pdir = vmm_create_pagedir();

    //Copy args into kernel memory
    uint32_t argc = 0;

    if(args != 0) {
        while(args[argc] != 0) {
            argc++;
        }
    }

    argc++;
    char** kargs = malloc((sizeof(char*)) * (argc + 1));

    kargs[0] = strclone(path);

    for(uint32_t i = 1; i < argc; i++) {
        kargs[i] = malloc(strlen(args[i-1]) + 1);
        strcpy(kargs[i], args[i-1]);
    }

    kargs[argc] = 0;

    //Execute file
    struct res_handle* handle = vfs_open(path, FM_EXEC | FM_READ);

    if(!handle) {
        vfs_set_error(PE_PERM_DENIED);
        free(path);
        return 0;
    }

    uint32_t size = vfs_available(handle);
    if(size == 0) {
        vfs_set_error(PE_CORRUPT_FILE);
        free(path);
        return 0;
    }

    void* modsrc = malloc(size);

    uint32_t res = vfs_read(handle, modsrc, size, 1);

    if(res != RW_OK) {
        free(path);
        free(modsrc);
        vfs_set_error(PE_FILESYSTEM);
        return 0;
    }

    uint32_t old_pdir = vmm_get_current_pagedir();

    //**********************************************************************************************************
    vmm_activate_pagedir(elf_mod_pdir);

    struct elf_header* header = modsrc;
    struct elf_program_header* ph;

    /* Ist es ueberhaupt eine ELF-Datei? */
    if (header->magic != ELF_MAGIC) {
        free(modsrc);
        free(path);
        vfs_set_error(PE_CORRUPT_FILE);
        return 0;
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

    //Copy args into new task
    char** usargs = vmm_alloc_ucont(1);

    for(uint32_t i = 0; i < argc; i++) {
        usargs[i] = vmm_alloc_ucont(1); //FIXME will fail on strings > 4095 chars or more than 1023 arguments
        strcpy(usargs[i], kargs[i]);
        free(kargs[i]);
    }

    usargs[argc] = 0;
    free(kargs);

    struct task* task = init_task(elf_mod_pdir, elf_mod_entry);
    if(get_current_task() != 0) {
        task->stdin = get_current_task()->stdin;
        task->stdout = get_current_task()->stdout;
        task->stderr = get_current_task()->stderr;
        task->execPath = get_current_task()->execPath;
    }

    if(execPath != 0) {
        task->execPath = execPath;
    }

    if(stdin != 0) {
        if(vfs_exists(stdin)) {
            struct res_handle* f = vfs_open(stdin, FM_READ);
            if(f) task->stdin = f;
        }
    }

    if(stdout != 0) {
        if(vfs_exists(stdout)) {
            struct res_handle* f = vfs_open(stdout, FM_READ);
            if(f) task->stdout = f;
        }
    }

    if(stderr != 0) {
        if(vfs_exists(stderr)) {
            struct res_handle* f = vfs_open(stderr, FM_READ);
            if(f) task->stderr = f;
        }
    }


    //TODO change stdio if requested and free kargs

    task->args = usargs;
    task->filePath = path;

    vmm_activate_pagedir(old_pdir);

    free(modsrc);

    return task->PID;
}

char* vfs_resolve_path(char* path) {
    struct res_node* node = vfs_get_node(path);
    if(node == 0) return 0;
    return vfs_construct_absolute_path_for_node(node);
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
