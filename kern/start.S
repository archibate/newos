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
	resb 32768
global boot_stack_top
boot_stack_top:
