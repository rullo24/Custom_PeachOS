ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS 
{
    . = 1M; /* kernel loaded into 1M of memory --> kernel to be loaded after this mb */
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