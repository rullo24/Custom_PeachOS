section .asm

global idt_load ; export symbol
idt_load:
    push ebp
    mov ebp, esp ; reference to frame

    mov ebx, [ebp+8] ; ebp+8 to skip over ebp just pushed to stack and ret addr (start at first argument)
    lidt [ebx]

    pop ebp
    ret