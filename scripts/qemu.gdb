#!/usr/bin/cgdb -x
# vim: ft=gdb ts=4 sts=4 tw=4

define cn
		file kernel
		target remote localhost:1234
end
cn

define toboot
	set architecture i8086
	break *0x7c00
	continue
end

define bc
	break $arg0
	continue
end

define q
	kill
	quit
end

define u
	x/10i $pc
end

bc exec.c:114
bc execve_inode
bc *0x1000127
bc *0x1000140
