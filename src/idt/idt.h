#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_desc {
    uint16_t offset_1; // offset bits (0 - 15)
    uint16_t selector; // selector thats in our GDT
    uint8_t zero; // does nothing --> unused
    uint8_t type_attr; // desc type and attrs
    uint16_t offset_2; // offset bits (16 - 31)
} __attribute__((packed)); // each member of struct is placed to minimise memory required

struct idtr_desc {
    uint16_t limit; // size of desc table - 1
    uint32_t base; // base addr of the start of the interrupt desc table
} __attribute__((packed));

void idt_init();
void enable_interrupts();
void disable_interrupts();

#endif