#!/usr/bin/cgdb -x
# vim: ft=gdb ts=4 sts=4 tw=4

add-symbol-file build/vmlinux

target remote localhost:1234

def lu
		add-symbol-file build/usr/$arg0
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
