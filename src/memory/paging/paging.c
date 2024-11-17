#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

void paging_load_directory(uint32_t *directory);
static uint32_t *current_directory = 0; // static means cannot access outside of this file

// function for paging --> aligning virtual mem w/ physical mem
struct paging_4gb_chunk *paging_new_4gb(uint8_t flags) {
    uint32_t *directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    
    // loop through entire page directory entry dir table --> create table for each
    for (int i=0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++) {
        uint32_t *entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int b=0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++) {
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE); // after finishing first table
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITABLE; // want page table to be writable (indiv pages might not be writable)
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_switch(uint32_t *directory) {
    paging_load_directory(directory);
    current_directory = directory;
}

uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk) {
    return chunk->directory_entry;
}

bool paging_is_aligned(void *addr) {
    return ((uint32_t)addr % PAGING_PAGE_SIZE == 0);
}

int paging_get_indexes(void *virtual_address, uint32_t *directory_index_out, uint32_t *table_index_out) {
    int res = 0;
    if (!paging_is_aligned(virtual_address)) {
        res = -EINVARG;
        goto out;
    }

    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
    
out:
    return res;
}

// set page table entry to value provided
int paging_set(uint32_t *directory, void *virt, uint32_t val) {
    if (!paging_is_aligned(virt)) {
        return -EINVARG;       
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);
    if (res < 0) { // error occurred
        return res;
    }

    uint32_t entry = directory[directory_index];

    // extracting only address (top 20-bits of paging table) --> no flags
    uint32_t *table = (uint32_t*)(entry & 0xfffff000); // lower bits (flags) are set to 1 when page is 4096 aligned (as it is in our table)
    table[table_index] = val;

    return 0;
}