; vim: ft=nasm ai

global __signal_restorer
__signal_restorer:
	add esp, 12
	pop eax
	pop ecx
	pop edx
	popfd
	ret
