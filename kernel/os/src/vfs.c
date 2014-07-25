#include "vfs.h"

struct rs_node* root = 0;
struct rs_node* current = 0;
struct rs_node* temp = 0;

static struct rs_node* vfs_find_node(struct rs_node* parent, char* name) {
    if(parent->res_type == RES_NLIST) {
        struct rs_node* ptr = parent->res_ptr;

        while(ptr != 0) {
            if(strcmp(name, ptr->name) == 0) {
                return ptr;
            }

            ptr = ptr->next;
        }
    }

    return 0;
}

static void vfs_insert_node(struct rs_node* parent, struct rs_node* child) {
    if(parent->res_type == RES_NLIST) {
        child->next = parent->res_ptr;
        child->parent = parent;
        parent->res_ptr = child;
    }
}

//TODO: create files :)

void vfs_init_root() {
    if(root == 0) {
        root = malloc(sizeof(struct rs_node));
        root->next = 0;
        root->parent = 0;
        strcpy(root->name, "$");
        root->res_ptr = 0;
        root->res_type = RES_NLIST;
    }
}

void vfs_debug_ls(char* path) {
    char* sub;
    struct rs_node* parent = root;
    struct rs_node* child = 0;

    sub = strtok(path, "/");
    while (sub != NULL)
    {
        child = vfs_find_node(parent, sub);
        parent = child;
        sub = strtok(0, "/");
    }

    child = child->res_ptr;

    while(child != 0) {
        kprintf("LS %s: %s\n", path, child->name);

        child = child->next;
    }
}

int vfs_create_path(char* path, uint32_t flags) {
    char* sub;
    struct rs_node* parent = root;
    struct rs_node* child = 0;

    int created = 0;

    sub = strtok(path, "/");
    while (sub != NULL)
    {
        kprintf("vfscp at %s child=%x, parent=%x, root=%x\n", sub, child, parent, root);
        child = vfs_find_node(parent, sub);
        if(child == 0) {
            child = malloc(sizeof(struct rs_node));

            strcpy(child->name, sub);
            child->res_type = RES_NLIST;
            child->res_ptr = 0;

            created++;

            kprintf("vfscp created %s", sub);

            vfs_insert_node(parent, child);
        }

        parent = child;

        sub = strtok(0, "/");
    }

    return created;
}
