#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "multiboot.h"

#define PTE_PRESENT 0x1
#define PTE_WRITE   0x2
#define PTE_USER    0x4

struct vmm_context {
    uint32_t** pagedir;
};

void vmm_init(struct multiboot_info* mb_info);
struct vmm_context* vmm_create_context(void);

#endif
