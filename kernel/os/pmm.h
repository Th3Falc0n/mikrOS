#ifndef PMM_H
#define PMM_H

#include "stdint.h"
#include "multiboot.h"
#include "types.h"
#include "stddef.h"

void* pmm_alloc();
void pmm_free(void* addr);
void pmm_init(struct multiboot_info* mb_info);

#endif
