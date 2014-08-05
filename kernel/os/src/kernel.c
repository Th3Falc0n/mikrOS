#include "kernel.h"
#include "vfs.h"
#include "ramfs/fifo.h"
#include "ramfs/block.h"
#include "ramfs/tar.h"
#include "ramfs/vgacntrl.h"

struct cpu_state* syscall(struct cpu_state* cpu) {
    save_cpu_state(cpu);

    cpu = get_current_task()->cpuState;

	switch (cpu->eax) {
	case 1: /* exit */
		cpu = terminate_current(cpu);
		break;

	case 2: /* fork */
	{

		uint32_t forkpdir = vmm_fork_current();

		struct task* ntask = init_task(forkpdir, 0);
		fork_task_state(ntask);

		cpu->eax = ntask->PID;
	}
        break;

    case 3: /* exec */
    {
        vfs_exec((char*) cpu->ebx, (char**) cpu->ecx, get_current_task());
    }
        break;

    case 4: /* getargs */
    {
        cpu->eax = (uint32_t) get_current_task()->args;
    }
        break;

    case 5: /* yield */
    {
        cpu = schedule(cpu);
    }
        break;

	case 10: /* fopen */
	{
	    char* name = strclone((char*) cpu->ebx);
	    uint32_t fmode = (uint32_t) cpu->ecx;

	    struct res_handle* handle = vfs_open(name, fmode);
	    if(handle) {
	        register_handle(handle);
	        cpu->eax = (uint32_t) handle;
	    }
	    else
	    {
	        cpu->eax = 0;
	    }

	    free(name);
	}
	    break;

	case 11: /* fclose */
	{
	    struct res_handle* handle = (void*) cpu->ebx;
	    if(!unregister_handle(handle)) {
	        vfs_close(handle);

	        cpu->eax = 0;
	    }
	    else
	    {
	        cpu->eax = (uint32_t) -1;
	    }
	}
	    break;

	case 12: /* fwrite */
	{
	    struct res_handle* handle = (void*) cpu->ebx;
	    if(handle != 0) {
	        cpu->eax = vfs_write(handle, (char*) cpu->ecx, cpu->edx, 1);
	    }
	    else
	    {
            cpu->eax = RW_ERR_VFS;
	    }
	}
	    break;

	case 13: /* fread */
	{
        struct res_handle* handle = (void*) cpu->ebx;
        if(handle != 0) {
            cpu->eax = vfs_read(handle, (char*) cpu->ecx, cpu->edx, 1);
        }
        else
        {
            cpu->eax = RW_ERR_VFS;
        }
	}
	    break;

	case 14: /* fmkfifo */
	{
        char* name = strclone((char*) cpu->ebx);
        vfs_create_kfile(name, ramfs_fifo_driver_struct(), &(uint32_t){4096}); //default to 4k Buffer-size

        struct res_handle* handle = vfs_open(name, FM_READ | FM_WRITE);
        if(handle) {
            register_handle(handle);
            cpu->eax = (uint32_t) handle;
        }
        else
        {
            cpu->eax = 0;
        }

        free(name);
	}
	    break;

	case 20: /* getpmhandle */
	{
	    struct res_handle* handle = 0;

	    switch(cpu->ebx) {
	    case PMID_STDOUT:
	        handle = get_current_task()->stdout;
	        break;
        case PMID_STDIN:
            handle = get_current_task()->stdin;
            break;
        case PMID_STDERR:
            handle = get_current_task()->stderr;
            break;
        default:
            handle = get_current_task()->stdout;
            break;
	    }

	    cpu->eax = (uint32_t) handle;
	}
	    break;

	case 21: /* fopenpmhandle */
	{
	    char* path = strclone((char*)cpu->ecx);

	    struct res_handle* open;
	    uint32_t fm = FM_WRITE;

	    if(cpu->ebx == PMID_STDIN) {
	        fm = FM_READ;
	    }

	    open = vfs_open(path, fm);

	    free(path);

	    if(!open) {
	        cpu->eax = (uint32_t) -1;
	        break;
	    }

	    struct res_handle* oldhandle = 0;

        switch(cpu->ebx) {
        case PMID_STDOUT:
            oldhandle = get_current_task()->stdout;
            get_current_task()->stdout = open;
            break;
        case PMID_STDIN:
            oldhandle = get_current_task()->stdin;
            get_current_task()->stdin = open;
            break;
        case PMID_STDERR:
            oldhandle = get_current_task()->stderr;
            get_current_task()->stderr = open;
            break;
        default:
            oldhandle = get_current_task()->stdout;
            get_current_task()->stdout = open;
            break;
        }

        if(oldhandle != 0) {
            vfs_close(oldhandle);
        }

        cpu->eax = 0;
	}
	    break;

	case 201: /* kputc */
		cpu->eax = kprintf("%c", cpu->ebx);
		break;

	case 202: /* kputs */
		cpu->eax = kprintf("%s", cpu->ebx);
		break;

	case 203: /* vmm_alloc_ucont */
		cpu->eax = (uint32_t) vmm_alloc_ucont(cpu->ebx);
		break;

	case 204: /* vmm_free */
		cpu->eax = 0;
		if (cpu->ebx >= PROGRAM_BOTTOM) { //Only in PROGRAM AREA ;)
			vmm_free((void*) cpu->ebx);
		}
		break;

	case 205: /* pmm_print_stats */
		pmm_print_stats();
		break;

	default:
		kprintf("Invalid Syscall %d...", cpu->eax);
		break;
	}

	return cpu;
}

void kernel_main(struct multiboot_info* mb_info) {
	uint32_t kernel_init_pdir = vmm_init();

	kprintf("Setting PIT interval...\n");

	outb(0x43, 0x36);
	outw(0x40, 1000);

    kprintf("Initializing vfs...\n");

    vfs_init_root();
    ramfs_fifo_init();
    ramfs_block_init();

    map_address_active((uint32_t) mb_info,
                       (uint32_t) mb_info, 0);
    map_address_active((uint32_t) mb_info->mi_mods_addr,
                       (uint32_t) mb_info->mi_mods_addr, 0);

    if (mb_info->mi_flags & MULTIBOOT_INFO_HAS_MODS) {
        vmm_map_range(mb_info->mi_mods_addr[0].start,
                      mb_info->mi_mods_addr[0].start,
                      mb_info->mi_mods_addr[0].end - mb_info->mi_mods_addr[0].start,
                      0);

        kprintf("Assuming mbmod[0] is a tarball (%d bytes) and unpacking it... \n", mb_info->mi_mods_addr[0].end - mb_info->mi_mods_addr[0].start);
        kprintf("Mapped mod from %x to %x\n", mb_info->mi_mods_addr[0].start, mb_info->mi_mods_addr[0].end);

        tar_load_ramfs(mb_info->mi_mods_addr[0].start);
    } else {
        kprintf("[PANIC] No multiboot module (initrfs) available.\n");
    }

    kprintf("[kernel_res] Creating /dev/vga\n");
    vfs_create_kfile("/dev/vga", ramfs_vga_driver_struct(), 0);

    if(vfs_exists("/ibin/init")) {
        kprintf("[init] /ibin/init found. Executing...\n");

        vfs_exec("/ibin/init", 0, 0);
        enableScheduling();
    }

	while(1);
    //*********************************************************************** KERNEL END
}
