[BITS 32]

global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ebp, 0x00200000
	mov esp, ebp

	; enable the A20 line
	in al, 0x92
	or al, 2
	out 0x92, al

	; remap the master PIC
	mov al, 00010001b
	out 0x20, al ; tell master PIC
	mov al, 0x20 ; interrupt 0x20 is where master ISR should start
	out 0x21, al
	mov al, 00000001b
	out 0x21, al
	; end of remap of the master PIC

	; enable interrupts
	sti

	call kernel_main
	jmp $

; ensuring that kernel code is aligned w/ upcoming C code
times 512-($ - $$) db 0 ; aligning w/ 512 bytes (16 bits divides into 512 perfectly)