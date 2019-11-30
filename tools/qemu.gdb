#!/usr/bin/cgdb -x
# vim: ft=gdb ts=4 sts=4 tw=4

def cn
		target remote localhost:1234
end

def i16
set architecture i8086
end

def ia
set disassembly-flavor intel
end

def da
set disassembly-next-line on
end

def lk
		add-symbol-file build/vmlinux
end

def lu
		add-symbol-file build/usr/$arg0
end
def ll
		add-symbol-file build/$arg0.dl
end

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

cn
ia
lk
lu busybox
ll libc
