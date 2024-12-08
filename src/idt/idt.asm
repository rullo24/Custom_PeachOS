section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global int21h
global idt_load ; export symbol
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp
    mov ebp, esp ; reference to frame

    mov ebx, [ebp+8] ; ebp+8 to skip over ebp just pushed to stack and ret addr (start at first argument)
    lidt [ebx]

    pop ebp
    ret

int21h:
    pushad   
    call int21h_handler
    popad
    iret

no_interrupt:
    pushad   
    call no_interrupt_handler
    popad
    iret

isr80h_wrapper:
    ; push general purpose regs to stack
    pushad

    ; push stack ptr so that we are pointing to the interupt frame
    push esp
    
    ; EAX holds our command --> push to stack for isr80h_handler
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8

    ; restore general purpose regs for user land
    popad
    mov eax, [tmp_res]
    iretd

section .data
; return result from isr80h_handler stored
tmp_res: dd 0

