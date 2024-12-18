[BITS 32]

global print:function ; ensure that elf filetype is capable of functions

print:
    push ebp
    mov ebp, esp 
    push dword[ebp+8]
    mov eax, 1 ; command print
    int 0x80 
    add esp, 4
    pop ebp
    ret
