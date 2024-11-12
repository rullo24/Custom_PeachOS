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
	jmp CODE_SEG:0x0100000 

ata_lba_read:
	mov ebx, eax, ; backup the LBA
	; send highest 8 bits of LBA to hard disk controller
	shr eax, 24 ; shift eax reg 24 bits to right (eax will now contain highest 8 bits of LBA)
	or eax, 0xe0 ; select the master drive
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

	; send more bits of the LBA
	mov dx, 0x1f4
	mov eax, ebx ; restore the backup LBA
	shr eax, 8 ; shifting eax to the right by 8 bits
	out dx, al
	; finished sending more bits of the LBA

	; send upper 16 bits of the LBA
	mov dx, 0x1f5
	mov eax, ebx ; restore the backup LBA
	shr eax, 16 ; moving eax to right by 16 bits
	out dx, al
	; finished sending upper 16 bits of the LBA

	mov dx, 0x1f7
	mov al, 0x20
	out dx, al

; read all sectors into memory
.next_sector:
	push ecx ; push to stack to save for later

; checking if we need to read
.try_again:
	mov dx, 0x1f7 ; reading from port 0x1f7 into al reg
	in al, dx 
	test al, 8
	jz .try_again ; jumping back to try_again if fail occurs

; we need to read 256 words at a time
	mov ecx, 256
	mov dx, 0x1f0
	rep insw ; do the insw instruction 256 times (uses ecx reg) --> 512 bytes (one sector)
	pop ecx ; restoring sector number
	loop .next_sector ; go to next_sector (decrement ecx - one sector less to read)
	; end of reading sectors into memory
	ret ; return from subroutine

times 510-($ - $$) db 0
dw 0xAA55
