#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"

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

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }

    return len;
}

void print(const char *str) {
    size_t len = strlen(str);
    for (int i=0; i<len; i++) {
        terminal_writechar(str[i], 15);
    }
}

// main runtime that all functions work in
void kernel_main() {
    terminal_init();
    print("Hello World!\ntest");

    // init the interrupt descriptor table
    idt_init();

    outb(0x60, 0xff);
}