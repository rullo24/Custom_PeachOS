ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS 
{
    . = 1M; /* kernel loaded into 1M of memory --> kernel to be loaded after this mb */
    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}