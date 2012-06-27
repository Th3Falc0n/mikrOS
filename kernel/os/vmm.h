#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "multiboot.h"

#define PTE_PRESENT 0x1
#define PTE_WRITE   0x2
#define PTE_USER    0x4

struct vmm_context {
    uint32_t* pagedir;
    uint32_t* vpagedir;
		uintptr_t alloc_offset;
};

void vmm_set_alloc_offset(struct vmm_context* context, uintptr_t offset);
void* vmm_alloc(struct vmm_context* context, uint16_t count);
struct vmm_context* vmm_create_context(void);
void vmm_map_kernel(struct vmm_context* context);
int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys, uint32_t flags);
void vmm_activate_context(struct vmm_context* context);
struct vmm_context* vmm_init(struct multiboot_info* mb_info);

#endif
