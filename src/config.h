#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define PEACHOS_TOTAL_INTERRUPTS 0x200 // 512 interrupts

#define PEACHOS_HEAP_SIZE_BYTES 104857600 // size of heap (requires PC w/ at least 100MB of RAM)
#define PEACHOS_HEAP_BLOCK_SIZE 4096
#define PEACHOS_HEAP_ADDRESS 0x01000000
#define PEACHOS_HEAP_TABLE_ADDRESS 0x00007e00 // 450KB of memory after this

#endif