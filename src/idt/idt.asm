section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load ; export symbol
global no_interrupt

idt_load:
    push ebp
    mov ebp, esp ; reference to frame

    mov ebx, [ebp+8] ; ebp+8 to skip over ebp just pushed to stack and ret addr (start at first argument)
    lidt [ebx]

    pop ebp
    ret

int21h:
    cli
    pushad   
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    cli
    pushad   
    call no_interrupt_handler
    popad
    sti
    iret

