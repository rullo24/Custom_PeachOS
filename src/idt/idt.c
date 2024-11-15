#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc *ptr);

void idt_zero() {
    print("Divide by zero error\n");
}

void idt_set(int interrupt_no, void *address) {
    struct idt_desc *desc = &idt_descriptors[interrupt_no]; // getting the addr of IDT descriptor
    desc->offset_1 = (uint32_t)address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE; // considering other bits (DPL, etc)
    desc->offset_2 = (uint32_t)address >> 16; // shifted because offset_2 is higher part of offset
}

void idt_init() {
    memset(idt_descriptors, 0x0, sizeof(idt_descriptors)); // creating NULL descriptors
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    idt_set(0, idt_zero);

    // load the interrupt descriptor table
    idt_load(&idtr_descriptor);
}