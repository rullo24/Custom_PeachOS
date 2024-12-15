ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
SECTIONS 
{
    . = 0x400000; 
    .text : ALIGN(4096) /* ensure everything in this section is aligned */
    {
        *(.text)
    }

    .asm : ALIGN(4096) /* not aligned w/ C code but won't interfere w/ it as it run last */
    {
        *(.asm)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }

}