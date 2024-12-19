[BITS 32]

global print:function ; ensure that elf filetype is capable of functions
global getkey:function
global peachos_malloc:function

; void print(const char *message);
print:
    push ebp
    mov ebp, esp 
    push dword[ebp+8]
    mov eax, 1 ; command print
    int 0x80 
    add esp, 4
    pop ebp
    ret

; int getkey();
getkey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; command getkey
    int 0x80
    pop ebp
    ret

; void *peachos_malloc(size_t size);
peachos_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; command malloc allocates mem for the process
    push dword[ebp+8] ; variable "size" as argument
    int 0x80
    add esp, 4
    pop ebp
    ret