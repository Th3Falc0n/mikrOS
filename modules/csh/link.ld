/*  Bei _start soll die Ausfuehrung losgehen */
ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

/*
 * Hier wird festgelegt, in welcher Reihenfolge welche Sektionen in die Binary
 * geschrieben werden sollen
 */
SECTIONS
{
    /* Das Programm wird an 0x10000000 geladen */
    . = 0x10000000;

    .text : {
        *(.text)
    }
    .data ALIGN(4096) : {
        *(.data)
    }
    .rodata ALIGN(4096) : {
        *(.rodata)
    }
    .bss ALIGN(4096) : {
        *(.bss)
    }
}


