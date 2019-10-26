; vim: ft=nasm ai

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
