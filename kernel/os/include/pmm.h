#ifndef PMM_H
#define PMM_H

#include "stdint.h"
#include "multiboot.h"
#include "types.h"
#include "stddef.h"
#include "string.h"
#include "console.h"

void*    pmm_alloc          ();
void     pmm_free           (void* addr);
void     pmm_init           (struct multiboot_info* mb_info);
uint32_t pmm_get_free_space (uint32_t divisor);
void     pmm_print_stats    ();

#endif
