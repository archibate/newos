; cont.asm

global switch_context
switch_context:
	mov [ecx + 0], esp
	mov [ecx + 4], ebx
	mov [ecx + 8], esi
	mov [ecx + 12], edi
	mov [ecx + 16], ebp
	pushfd
	pop dword [ecx + 20]
	mov esp, [edx + 0]
	mov ebx, [edx + 4]
	mov esi, [edx + 8]
	mov edi, [edx + 12]
	mov ebp, [edx + 16]
	push dword [edx + 20]
	popfd
	ret
