; vim: ft=nasm ai

MB_MAGIC	equ	0x1BADB002
MB_FLAGS	equ	0
	dd	MB_MAGIC
	dd	MB_FLAGS
	dd	-(MB_MAGIC + MB_FLAGS)

section .text
global _start
extern main
_start:
	mov byte [0xb8000], 'O'
	mov byte [0xb8002], 'k'
	xor ebp, ebp
	mov esp, boot_stack_top
	call main
	cli
	hlt

section .bss
	resb 16384
boot_stack_top:
