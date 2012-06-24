#include "pmm.h"
#include "string.h"
#include "console.h"

#define BITMAP_SIZE 32768

uint32_t allocatable[BITMAP_SIZE];

void* pmm_alloc() {
	int i;
	for(i = 0; i < BITMAP_SIZE; i++) {
		if(allocatable[i]) {
			int s;
			for(s = 0; s < 32; s++) {
				if(allocatable[i] & (1 << s)) {
					allocatable[i] &= ~(1 << s);
					kprintf("Allocated: %x (%x) \n", ((i * 32 + s) * 4096), (i * 32 + s));
					return (uint8_t*)((i * 32 + s) * 4096);
				}
			}
		}	
	}
	return NULL;
}

void pmm_free(void* addr) {
	uintptr_t ptr = ((uintptr_t)addr) / 4096;

	uintptr_t s = ptr % 32;
	uintptr_t i = ptr / 32;

	allocatable[i] |= (1 << s);
}

void pmm_mark_used(void* addr)  {
	uintptr_t ptr = ((uintptr_t)addr) / 4096;

	uintptr_t s = ptr % 32;
	uintptr_t i = ptr / 32;

	allocatable[i] &= ~(1 << s);
}

void pmm_init(struct multiboot_info* mb_info) {
	struct multiboot_mmap* mmap = mb_info->mi_mmap_addr;
	struct multiboot_mmap* mmap_end = (void*)
		  ((uintptr_t) mb_info->mi_mmap_addr + mb_info->mi_mmap_length);

	memset(allocatable, 0, sizeof(allocatable));

	kprintf("Mem-Bitmap: %x to %x\n", allocatable, &(allocatable[BITMAP_SIZE]));
	 
	while (mmap < mmap_end) {
		  if (mmap->mm_type == 1) {
		      uintptr_t addr = mmap->mm_base_addr;
		      uintptr_t end_addr = addr + mmap->mm_length;
	 
		      while (addr < end_addr) {
		          pmm_free((void*) addr);
		          addr += 0x1000;
		      }
		  }
		  mmap++;
	}

	extern const void kernel_start;
	extern const void kernel_end;
	 
	uintptr_t addr = (uintptr_t) &kernel_start;
	while (addr < (uintptr_t) &kernel_end) {
		  pmm_mark_used((void*) addr);
		  addr += 0x1000;
	}

  struct multiboot_module* modules = mb_info->mi_mods_addr;

  pmm_mark_used(mb_info);
  pmm_mark_used(modules);

  int i;
  for (i = 0; i < mb_info->mi_mods_count; i++) {
      addr = (uintptr_t)modules[i].start;
      while (addr < (uintptr_t)modules[i].end) {
          pmm_mark_used((void*) addr);
          addr += 0x1000;
      }
  }
}
