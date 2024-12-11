[BITS 32]

section .asm

global _start

_start:
    ; wait for key press
    call getkey 

    ; output message to us
    push message
    mov eax, 1
    int 0x80
    add esp, 4
    jmp $

; continues until a key press is returned to us
getkey:
    mov eax, 2 ; command getkey
    int 0x80
    cmp eax, 0x0
    je getkey
    ret

section .data
message: db 'Hello World!', 0x0
