#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "multiboot.h"
#include "pmm.h"
#include "console.h"

#define PD_EXIST   (1 << 0)
#define PD_WRITE   (1 << 1)
#define PD_PUBLIC  (1 << 2)
#define PD_WTCACHE (1 << 3)
#define PD_NOCACHE (1 << 4)
#define PD_USED    (1 << 5)
#define PD_USER0   (1 << 9)
#define PD_USER1   (1 << 10)
#define PD_USER2   (1 << 11)

#define PT_EXIST   (1 << 0)
#define PT_WRITE   (1 << 1)
#define PT_PUBLIC  (1 << 2)
#define PT_WTCACHE (1 << 3)
#define PT_NOCACHE (1 << 4)
#define PT_USED    (1 << 5)
#define PT_WRITTEN (1 << 6)
#define PT_USER0   (1 << 9)
#define PT_USER1   (1 << 10)
#define PT_USER2   (1 << 11)

#define ACTIVE_CONTEXT_VADDR 0x2000000
#define STATIC_ALLOC_VADDR 0x1900000
#define USERSPACE_ZERO 0x10000000

struct vmm_context {
    uint32_t* pagedir;
    
    uint32_t alloc_addr_count;
    uint32_t alloc_byte_count;
};

uint32_t vmm_init(void);
uint32_t vmm_create_pagedir(void);
void     vmm_activate_pagedir(uint32_t context);
void*    vmm_alloc(uint32_t* paddr);
void*    vmm_alloc_static(uint32_t vaddr);
uint32_t vmm_get_current_pagedir(void);
void     map_address_context(uint32_t* pagedir, uint32_t vaddr, uint32_t paddr);
void     map_address_active(uint32_t vaddr, uint32_t paddr);  


#endif
