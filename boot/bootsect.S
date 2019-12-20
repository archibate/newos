; vim: ft=nasm ai

BOOT_DRV  equ 0x80
%include "build/boot/kerninfo.inc"
tmp_kaddr equ 0x10000

	org 0x7c00
	bits 16
_start:
	mov si, load_msg
	call print
	call read_sect
	cmp word [stage2_signature], 0x55aa
	jne error
	jmp stage2

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

read_sect:
	mov si, disk_addr_packet
	mov ah, 0x42
	mov dl, BOOT_DRV
	int 0x13
	ret

load_msg:
	db "Loading System...", 0
error_msg:
	db "ERROR", 0

disk_addr_packet:
	db	16	; [0] packet size: 16 bytes
	db	0	; [1] reserved, always be 0
	dw	1	; [2] blocks to read
	dw	0	; [4] transfer buffer: offset
	dw	0x07e0	; [6] transfer buffer: segment
	dq	1	; [8] starting block LBA

	times 510-($-$$) db 0
	dw 0xaa55

stage2:
%ifdef _VIDEO
	call set_video
%endif
	mov si, disk_addr_packet
	mov word [si + 8], 4
	mov word [si + 6], tmp_kaddr >> 4
	mov cx, kern_size >> 9
.repeat:
	call read_sect
	mov si, disk_addr_packet
	inc word [si + 8]
	add word [si + 6], 512 >> 4
	loop .repeat
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

set_video:
	;jmp .vga13;;
	mov di, buffer
	mov ax, 0x4f00
	int 0x10
	cmp ax, 0x004f
	jne .vga13
	cmp word [di + 4], 0x0200
	jb .vga13
	mov cx, 0x118
	mov ax, 0x4f01
	int 0x10
	cmp ax, 0x004f
	jne .vga13
	test word [di], 0x0080
	jz .vga13
	mov ax, [di + 0x12]
	mov [vram_nx], ax
	mov ax, [di + 0x14]
	mov [vram_ny], ax
	mov al, [di + 0x19]
	mov [vram_bpp], al
	mov eax, [di + 0x28]
	mov [vram_addr], eax
	mov bx, 0x4118
	mov ax, 0x4f02
	int 0x10
	ret
.vga13:
	mov ax, 0x0013
	int 0x10
	jc error
	mov eax, [di + 0x28]
	mov [vram_addr], dword 0xa0000
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
	mov edi, kern_addr
	mov ecx, (kern_size + 3) >> 2
	cld
	rep movsd
	xor eax, eax
	mov ecx, (kmem_size + 3 - kern_size) >> 2
	rep stosd
	mov eax, [vram_addr]
	mov [eax + 40000], dword 0xffffffff
	mov [eax + 40004], dword 0xffffffff
	mov [eax + 40008], dword 0xffffffff
	mov [eax + 40012], dword 0xffffffff
	jmp 0x0008:kern_addr

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

done_msg:
	db "done", 13, 10, 0
buffer:

	times 1012-($-$$) db 0
vram_addr: ; access me at 0x7ff4
	dd 0
vram_nx:
	dw 320
vram_ny:
	dw 200
vram_bpp:
	dw 8
stage2_signature:
	dw 0x55aa

	times 2048-($-$$) db 0 ; reserved for NEFS super block
