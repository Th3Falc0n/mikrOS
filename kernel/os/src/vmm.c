#include "vmm.h"

extern const void kernel_start;
extern const void kernel_end;

uint32_t kernel_pagetables[PMEM_TABLES]; //TODO: kernel pagetables. u know this shit.

uint32_t* active_pagetables = (uint32_t*) (ACTIVE_PAGETBL_VADDR);
struct vmm_context* active_context = (void*) (ACTIVE_CONTEXT_VADDR);

uint32_t active_pagedir = 0;

static uint32_t* vmm_create_vpdraw(uint32_t* pagedir) {
	uint32_t phys_context = 0;
	struct vmm_context* context = vmm_alloc(&phys_context);

	uint32_t phys_pagedir = 0;
	context->pagedir = vmm_alloc(&phys_pagedir);
	if (pagedir != 0)
		*pagedir = phys_pagedir;

	uint32_t* pagedir_ptr = vmm_alloc(0);
	uint32_t paddr, i, i2;

	for (i = 0; i < 1024; i++) {
		pagedir_ptr[i] = (uint32_t) vmm_alloc(&paddr);

		if (i < PMEM_TABLES) {
			vmm_free((void*) (pagedir_ptr[i]));
			map_address_active(pagedir_ptr[i], kernel_pagetables[i], 0); //Can reuse pagedir_ptr[i] cause of vmm_free
			paddr = kernel_pagetables[i];
		}

		context->pagedir[i] = paddr | PD_PRESENT | PD_WRITE | PD_PUBLIC;

		if (i >= PMEM_TABLES) { //don't override kernel tables ;)
			for (i2 = 0; i2 < 1024; i2++) {
				uint32_t vaddr = (i << 22) + (i2 << 12);

				((uint32_t*) (pagedir_ptr[i]))[i2] = (
						(vaddr > ALLOCATABLE_BOTTOM) ? PT_ALLOCATABLE : 0)
						| ((vaddr > PROGRAM_BOTTOM) ? PT_PUBLIC : 0);
			}
		}
	}

	for (i = 0; i < 1024; i++) {
		paddr = context->pagedir[i] & 0xFFFFF000;
		map_address_context(pagedir_ptr,
				(uint32_t) active_pagetables + i * 0x1000, paddr, 0);
	}

	map_address_context(pagedir_ptr, (uint32_t) active_context, phys_context,
			0);

	for (i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
		map_address_context(pagedir_ptr, i, i, 0);
	}

	uint32_t* pd_ptr = context->pagedir;
	context->pagedir = (uint32_t*) phys_pagedir;

	vmm_unmap(pd_ptr);
	vmm_unmap(context);

	return pagedir_ptr;
}

static void vmm_free_pdptr(uint32_t* pagedir_ptr) {
	for (uint32_t i = 0; i < 1024; i++) {
		vmm_unmap((void*) pagedir_ptr[i]);
	}

	vmm_unmap(pagedir_ptr);
}

uint32_t vmm_get_current_pagedir(void) {
	return active_pagedir;
}

uint32_t vmm_fork_current(void) {
	uint32_t phys_pagedir, paddr, i;
	uint32_t* pdptr = vmm_create_vpdraw(&phys_pagedir);

	for (i = PROGRAM_BOTTOM; i < 0xFFFFF000; i += 0x1000) {
		if (active_pagetables[i >> 12] & PT_PRESENT) {
			void* newp = vmm_alloc_user(&paddr);

			memcpy(newp, (void*) i, 0x1000);
			map_address_context(pdptr, i, paddr, 0);

			vmm_unmap(newp);
		}

	}

	vmm_free_pdptr(pdptr);

	return phys_pagedir;
}

uint32_t vmm_create_pagedir() {
	uint32_t phys_pagedir;
	uint32_t* pdptr = vmm_create_vpdraw(&phys_pagedir);
	vmm_free_pdptr(pdptr);

	return phys_pagedir;
}

uint32_t vmm_resolve(void* vaddr) {
	return active_pagetables[(uint32_t) vaddr >> 12] & 0xFFFFF000;
}

uint32_t vmm_resolve_ppd(uint32_t pdir, void* vaddr) {
	uint32_t pd_entry = (uint32_t) vaddr >> 22;
	uint32_t pt_entry = ((uint32_t) vaddr >> 12) % 1024;

	uint32_t* vpd = vmm_alloc(0);
	vmm_free(vpd);

	map_address_active((uint32_t) vpd, pdir, 0);
	uint32_t table = vpd[pd_entry] & 0xFFFFF000;

	map_address_active((uint32_t) vpd, table, 0);
	uint32_t addr = vpd[pt_entry] & 0xFFFFF000;

	return addr & 0xFFFFF000;
}

void vmm_map_range(void* vaddr, void* paddr, uint32_t length, uint32_t flags) {
	if ((uint32_t) vaddr & 0xFFF)
		return;
	if ((uint32_t) paddr & 0xFFF)
		return;

	for (uint32_t i = 0; i < length; i += 0x1000) {
		map_address_active((uint32_t) vaddr + i, (uint32_t) paddr + i, flags);
	}
}

void map_address_context(uint32_t* pagedir, uint32_t vaddr, uint32_t paddr,
		uint32_t flags) {
	uint32_t pd_entry = vaddr >> 22;
	uint32_t pt_entry = (vaddr >> 12) % 1024;

	((uint32_t*) (pagedir[pd_entry] & 0xFFFFF000))[pt_entry] = (paddr
			& 0xFFFFF000) | PT_PRESENT | PT_WRITE | (flags & 0xFFF)
			| (((uint32_t*) (pagedir[pd_entry] & 0xFFFFF000))[pt_entry]
					& (PT_PUBLIC | PT_ALLOCATABLE));
}

void map_address_active(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
	active_pagetables[vaddr >> 12] = (paddr & 0xFFFFF000) | PT_PRESENT
			| PT_WRITE | (flags & 0xFFF)
			| (active_pagetables[vaddr >> 12] & (PT_PUBLIC | PT_ALLOCATABLE));
	asm volatile("invlpg %0" : : "m" (*(char*)vaddr));
}

void vmm_free(void* p_vaddr) {
	uint32_t vaddr = (uint32_t) p_vaddr;

	if ((active_pagetables[vaddr >> 12] & (PT_ALLOCATABLE | PT_PRESENT))
			== (PT_ALLOCATABLE | PT_PRESENT)) {
		pmm_free((void*) (active_pagetables[vaddr >> 12] & 0xFFFFF000));

		active_pagetables[vaddr >> 12] = PT_ALLOCATABLE
				| (active_pagetables[vaddr >> 12] & PT_PUBLIC);
		asm volatile("invlpg %0" : : "m" (*(char*)vaddr));
	}
}

void vmm_unmap(void* p_vaddr) { //USE ONLY IF YOU KNOW WHAT YOU DO. POTENTIAL MEMORY LEAK!
	uint32_t vaddr = (uint32_t) p_vaddr;

	if ((active_pagetables[vaddr >> 12] & (PT_ALLOCATABLE | PT_PRESENT))
			== (PT_ALLOCATABLE | PT_PRESENT)) {
		active_pagetables[vaddr >> 12] = PT_ALLOCATABLE
				| (active_pagetables[vaddr >> 12] & PT_PUBLIC);
		asm volatile("invlpg %0" : : "m" (*(char*)vaddr));
	}
}

static void* vmm_alloc_in_range(uint32_t low, uint32_t high, uint32_t* retpaddr,
		uint32_t cont) {
	void* vaddr = 0;
	uint32_t i = 0;
	uint32_t c = 0;

	for (i = (low & 0xFFFFF000); i < high; i += 0x1000) {
		if ((active_pagetables[i >> 12] & (PT_ALLOCATABLE | PT_PRESENT))
				== PT_ALLOCATABLE) {
			if (c == 0)
				vaddr = (void*) i;
			c++;
			if (c >= cont)
				break;
		} else {
			c = 0;
		}
	}

	uint32_t off = 0;

	while (c--) {
		vmm_alloc_addr(vaddr + off * 0x1000, off == 0 ? retpaddr : 0);
		off++;
	}

	return vaddr;
}

void* vmm_alloc_ucont(uint32_t cont) {
	return vmm_alloc_in_range(PROGRAM_BOTTOM, 0xFFFFF000, 0, cont);
}

void* vmm_alloc_user(uint32_t* retpaddr) {
	return vmm_alloc_in_range(PROGRAM_BOTTOM, 0xFFFFF000, retpaddr, 1);
}

void* vmm_alloc_cont(uint32_t cont) {
	return vmm_alloc_in_range(ALLOCATABLE_BOTTOM, USERSPACE_BOTTOM, 0, cont);
}

void* vmm_alloc(uint32_t* retpaddr) {
	return vmm_alloc_in_range(ALLOCATABLE_BOTTOM, USERSPACE_BOTTOM, retpaddr, 1);
}

void* vmm_alloc_addr(void* reqvaddr, uint32_t* retpaddr) {
	if (reqvaddr == 0) {
		kprintf(
				"Denied vmm_alloc_addr at %x (Flags: %x) ... this is a potential mm-fault \n",
				reqvaddr, 0);
		return 0;
	}

	if ((active_pagetables[(uint32_t) reqvaddr >> 12]
			& (PT_ALLOCATABLE | PT_PRESENT)) != PT_ALLOCATABLE) {
		kprintf(
				"Denied vmm_alloc_addr at %x (Flags: %x) ... this is a potential mm-fault \n",
				reqvaddr, active_pagetables[(uint32_t) reqvaddr >> 12]);
		return 0;
	}

	uint32_t paddr = (uint32_t) pmm_alloc();
	map_address_active((uint32_t) reqvaddr, paddr, PT_PUBLIC | PT_ALLOCATABLE);

	if (retpaddr != 0)
		*retpaddr = paddr;

	return reqvaddr;
}

void vmm_activate_pagedir(uint32_t pdpaddr) {
	active_pagedir = pdpaddr;
	asm volatile("mov %0, %%cr3" : : "r" (pdpaddr));
}

uint32_t vmm_init(void) {
	//CREATE CONTEXT ************************************************************

	struct vmm_context* context = pmm_alloc();
	uint32_t i, i2;

	context->pagedir = pmm_alloc();

	for (i = 0; i < 1024; i++) {
		context->pagedir[i] = ((uint32_t) pmm_alloc());
		if (i < PMEM_TABLES) {
			kernel_pagetables[i] = context->pagedir[i];
		}

		context->pagedir[i] |= PD_PRESENT | PD_WRITE | PD_PUBLIC;

		for (i2 = 0; i2 < 1024; i2++) {
			uint32_t vaddr = (i << 22) + (i2 << 12);

			((uint32_t*) (context->pagedir[i] & 0xFFFFF000))[i2] = (
					(vaddr > ALLOCATABLE_BOTTOM) ? PT_ALLOCATABLE : 0)
					| ((vaddr > PROGRAM_BOTTOM) ? PT_PUBLIC : 0);
		}
	}

	for (i = 0; i < 1024; i++) {
		uint32_t paddr = context->pagedir[i] & 0xFFFFF000;
		map_address_context(context->pagedir,
				(uint32_t) active_pagetables + i * 0x1000, paddr, 0);
	}

	map_address_context(context->pagedir, (uint32_t) active_context,
			(uint32_t) context, 0);

	for (i = 0x1000; i < (uint32_t) &kernel_end; i += 0x1000) {
		map_address_context(context->pagedir, i, i, 0);
	}

	//END CREATE CONTEXT ********************************************************

	vmm_activate_pagedir((uint32_t) context->pagedir);

	uint32_t cr0;

	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= (1 << 31);
	asm volatile("mov %0, %%cr0" : : "r" (cr0));

	return (uint32_t) active_context->pagedir;
}
