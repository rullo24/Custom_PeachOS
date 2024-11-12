ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start:
	jmp short start
	nop

times 33 db 0

start: 
	jmp 0:step2

step2:
	cli ; clear interrupts
	mov ax, 0x00
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00
	sti ; enables interrupts

.load_protected:
	cli
	lgdt[gdt_descriptor]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp CODE_SEG:load32
	jmp $

; GDT 
gdt_start:
gdt_null:
	dd 0x0
	dd 0x0

; offset 0x8
gdt_code:
	dw 0xffff
	dw 0
	db 0
	db 0x9a
	db 11001111b
	db 0

; offset 0x10
gdt_data:
	dw 0xffff
	dw 0
	db 0
	db 0x92
	db 11001111b
	db 0

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

; entering 32-bit mode again --> load geometry
[BITS 32]
load32:
	mov eax, 1 ; starting sector we want to load from
	mov ecx, 100 ; writing 100 sectors of nulls
	mov edi, 0x0100000 ; the equivalent of 1MB
	call ata_lba_read ; label to talk with the drive and load sectors into memory

ata_lba_read:
	mov ebx, eax, ; backup the LBA
	; send highest 8 bits of LBA to hard disk controller
	shr eax, 24 ; shift eax reg 24 bits to right (eax will now contain highest 8 bits of LBA)
	mov dx, 0x1f6 ; the port that we will write the bits to
	out dx, al ; al is 8-bits
	; finished sending the highest 8-bits of the LBA

	; send the total sectors to read
	mov eax, ecx
	mov dx, 0x1f2
	out dx, al
	; finished sending the total sectors to read

	; send more bits of the LBA
	mov eax, ebx ; restore the backup LBA
	mov dx, 0x1f3
	out dx, al ; talking w/ bus on motherboard --> controller listening to us
	; finished sending more bits of the LBA


times 510-($ - $$) db 0
dw 0xAA55
