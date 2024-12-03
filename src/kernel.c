#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "string/string.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "task/tss.h"
#include "gdt/gdt.h"
#include "config.h"

// global vars
uint16_t *video_mem = 0x0; // to output chars to screen, simply put them at 0xb8000 and 0xb8001 for colour
uint16_t terminal_row = 0x0;
uint16_t terminal_col = 0x0;

uint16_t terminal_make_char(char c, char colour) {
    return (colour << 8) | c; // 0x*colour**char* 8-bits == 1-byte
}

// pasting character in location within terminal
void terminal_putchar(int x, int y, char c, char colour) {
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

// incremental char pasting 
void terminal_writechar(char c, char colour) {
    if (c == '\n') {
        terminal_row += 1;
        terminal_col = 0;
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1; // incrementing location to avoid overwriting same location
    if (terminal_col >= VGA_WIDTH) { // checking if at end of row
        terminal_col = 0;
        terminal_row += 1;
    }
}

// loop through terminal and clear the C bios info
void terminal_init() {
    // predefined value for placing characters onto the screen
    video_mem = (uint16_t*)(0xB8000); // as per prev Section4:25
    terminal_row = 0x0;
    terminal_col = 0x0;

    for (int y=0; y<VGA_HEIGHT; y++) {
        for (int x=0; x <VGA_WIDTH; x++) {
            // converting (x, y) to an index to parse into array
            terminal_putchar(x, y, ' ', 0); // clearing all current slots
        }
    }

}

void print(const char *str) {
    size_t len = strlen(str);
    for (int i=0; i<len; i++) {
        terminal_writechar(str[i], 15);
    }
}

static struct paging_4gb_chunk *kernel_chunk = 0; // static means that the global is only accessible from kernel.c

void panic(const char *msg) {
    print(msg);
    while (1) {}
}

struct tss tss;
struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x0, .limit = 0x00, .type = 0x00}, // NULL segment
    {.base = 0x0, .limit = 0xffffffff, .type = 0x9a}, // kernel code segment
    {.base = 0x0, .limit = 0xffffffff, .type = 0x92}, // kernel data segment
    {.base = 0x0, .limit = 0xffffffff, .type = 0xf8}, // user code segment
    {.base = 0x0, .limit = 0xffffffff, .type = 0xf2}, // user data segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xe9} // TSS segment
};

// main runtime that all functions work in
void kernel_main() {
    terminal_init();
    print("Hello World!\ntest");

    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);
    
    // load the GDT
    gdt_load(gdt_real, sizeof(gdt_real));

    // init the heap
    kheap_init();

    // init the filesystems
    fs_init();

    // search and init the disks
    disk_search_and_init();

    // init the interrupt descriptor table
    idt_init();

    // setup TSS
    memset(&tss, 0x0, sizeof(tss)); // setting whole TSS to NULLs
    tss.esp0 = 0x600000; // kernel stack location
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // load the TSS
    tss_load(0x28); // 0x28 is offset in GDT real

    // setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    // enable paging
    enable_paging();

    // enable the system interrupts
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        struct file_stat s;
        fstat(fd, &s);
        fclose(fd);

        print("\ntesting\n");
    }

    while (1) {}
}