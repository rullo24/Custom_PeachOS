#include "gdt.h"
#include "kernel.h"

void encodeGdtEntry(uint8_t *target, struct gdt_structured source) {
    if ((source.limit > 65536) && (source.limit & 0xfff) != 0xfff) {
        panic("encodeGdtEntry: Invalid Argument\n");
    }

    target[6] = 0x40;
    if (source.limit > 65536) {
        source.limit = source.limit >> 12;
        target[6] = 0xc0;
    }

    // encode the limit
    target[0] = source.limit & 0xff;
    target[1] = (source.limit >> 8) & 0xff;
    target[6] |= (source.limit >> 16) & 0x0f;

    // encode the base
    target[2] = source.base & 0xff;
    target[3] = (source.base >> 8) & 0xff;
    target[4] = (source.base >> 16) & 0xff;
    target[7] = (source.base >> 24) & 0xff;
    
    // set the type
    target[5] = source.type;
}

void gdt_structured_to_gdt(struct gdt *gdt, struct gdt_structured *structured_gdt, int total_entries) {
    for (int i=0; i < total_entries; i++) {
        encodeGdtEntry((uint8_t*)&gdt[i], structured_gdt[i]); // structured gdt --> normal gdt
    }
}