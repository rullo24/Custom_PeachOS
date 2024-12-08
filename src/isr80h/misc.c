#include "misc.h"
#include "idt/idt.h"

void *isr80h_command0_sum(struct interrupt_frame *frame) {

    return 0x0; // parsed back to user space
}
