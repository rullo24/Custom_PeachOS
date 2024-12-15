#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

struct elf_file {
    char filename[PEACHOS_MAX_PATH];
    int in_memory_size;
    void *elf_memory; // ptr to phys addr where elf mem loaded
    void *virtual_base_address; 
    void *virtual_end_address;
    void *physical_base_address;
    void *physical_end_address;
};

#endif