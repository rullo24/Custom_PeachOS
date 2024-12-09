[BITS 32]

section .asm

global _start

_start:
    push message
    mov eax, 0x01 ; print command
    int 0x80
    add esp, 0x04
    jmp $

section .data
message: db 'Hello World!', 0x0
