; vim: ft=nasm ai

tmp_kaddr equ 0x10000
fin_kaddr equ 0x100000
kern_size equ 0x10000

	org 0x7c00
	bits 16
_start:
	jmp main

main:
	mov si, load_msg
	call print
	call read_kernel
	mov si, done_msg
	call print
enable_prot:
	cli
	lgdt [gdtr]
	lidt [idtr]
.a20:	in al, 0x64
	test al, 0x02
	jnz .a20
	mov al, 0xdf
	out 0x64, al
	mov ax, 0x01
	lmsw ax
	jmp 0x0008:start32

error:
	mov si, error_msg
	call print
halt:
	hlt
	jmp halt

print:
	mov ah, 0x0e
.repeat:
	lodsb
	test al, al
	jz .done
	int 0x10
	jmp .repeat
.done:
	ret

read_kernel:
	mov si, disk_addr_packet
	mov word [si + 6], tmp_kaddr >> 4
	mov cx, kern_size >> 9
.repeat:
	call read_sect
	mov si, disk_addr_packet
	inc word [si + 8]
	add word [si + 6], 512 >> 4
	loop .repeat
	ret

read_sect:
	mov si, disk_addr_packet
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	ret

	bits 32
start32:
	mov eax, 0x0010
	mov ss, eax
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov esi, tmp_kaddr
	mov edi, fin_kaddr
	mov ecx, kern_size >> 2
	cld
	rep movsd
	jmp 0x0008:fin_kaddr

load_msg:
	db "Loading System...", 0
done_msg:
	db "done", 13, 10, 0
error_msg:
	db "ERROR", 0

disk_addr_packet:
	db	0x10	; [0] packet size: 16 bytes
	db	0x00	; [1] reserved, always be 0
	dw	0x01	; [2] blocks to read
	dw	0x00	; [4] transfer buffer: offset
	dw	0x00	; [6] transfer buffer: segment
	dq	0x01	; [8] starting block LBA

	align 8
gdt:
	dq 0x0000000000000000
	dq 0x00cf9a000000ffff
	dq 0x00cf92000000ffff
gdtr:
	dw $-gdt-1
	dd gdt
idtr:
	dw 0
	dd 0

	times 510-($-$$) db 0
	dw 0xaa55
