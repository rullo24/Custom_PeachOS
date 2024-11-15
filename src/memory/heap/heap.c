#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

// objective: does this table know how many blocks we need between ptr and end?
static int heap_validate_table(void *ptr, void *end, struct heap_table *table) {
    int res = 0;
    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE; 
    if (table->total != total_blocks) { // miscalculated size
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

static bool heap_validate_alignment(void *ptr) {
    return ((unsigned int)ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0; // returns true if alignment
}

int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table) {
    int res = 0;   
    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end)) {
        res = -EINVARG; // negative status code
        goto out;
    }

    memset(heap, 0, sizeof(struct heap)); // zeroing all heap memory
    heap->saddr = ptr; // sets heap start address to pointer provided
    heap->table = table; // sets heap table

    res = heap_validate_table(ptr, end, table);
    if (res < 0) { // ensure table is correct size
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return res;
}

static uint32_t heap_align_value_to_upper(uint32_t val) {
    // aligned already 
    if (val % PEACHOS_HEAP_BLOCK_SIZE == 0) {
        return val;
    }

    val = (val - (val % PEACHOS_HEAP_BLOCK_SIZE)); // checking spare bytes from anti-alignment
    val += PEACHOS_HEAP_BLOCK_SIZE; // aligning up to the next block
    return val;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry) {
    return entry & 0x0f; // only return first 4 bits
}

int heap_get_start_block(struct heap *heap, uint32_t total_blocks) {
    struct heap_table *table = heap->table;
    int bc = 0; // store current block
    int bs = -1; // store block start (first free block)

    for (size_t i=0; i < table->total; i++) {
        // if entry we're on is NOT free --> reset state
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            bc = 0;
            bs = -1;
            continue;
        }

        // if this is the first block
        if (bs == -1) {
            bs = i; // know start block
        }
        bc++; // increment

        // found a start block with enough blocks afterwards for alloc
        if (bc == total_blocks) {
            break; 
        }
    }

    // out of memory --> could not alloc
    if (bs == -1) {
        return -ENOMEM;
    }

    return bs;
}

void *heap_block_to_address(struct heap *heap, int block) {
    return heap->saddr + (block * PEACHOS_HEAP_BLOCK_SIZE);
}

void heap_mark_blocks_taken(struct heap *heap, int start_block, int total_blocks) {
    int end_block = (start_block + total_blocks) - 1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if (total_blocks > 1) { // more than one block required --> continue looking for end
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block ; i <= end_block; i++) {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN; // make next entry as loop goes back around

        if (i != end_block - 1) { // continuing block search until at end block
            entry |= HEAP_BLOCK_HAS_NEXT; // flag within block to signify that there is more memory in the alloc
        }
    }

}

void *heap_malloc_blocks(struct heap *heap, uint32_t total_blocks) {
    void *address = 0;

    int start_block = heap_get_start_block(heap, total_blocks);
    if (start_block < 0) { // error occurred
        goto out;
    }

    address = heap_block_to_address(heap, start_block);

    // mark the blocks as taken
    heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
    return address;
}

void heap_mark_blocks_free(struct heap *heap, int starting_block) {
    struct heap_table *table = heap->table;
    
    // loop from starting block until end of table
    for (int i=starting_block; i < (int)table->total; i++) {

        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE; // set so that can be reused by malloc
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break; // have reached the end of the allocation
        }
    }
}

int heap_address_to_block(struct heap *heap, void *address) {
    return ((int)(address - heap->saddr)) / PEACHOS_HEAP_BLOCK_SIZE;
}

void *heap_malloc(struct heap *heap, size_t size) {
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;

    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap *heap, void *ptr) {
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}