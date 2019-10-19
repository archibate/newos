COPT=-ggdb -gstabs+
QEMUOPT=-serial stdio -display none
QEMUCMD=qemu-system-i386 -kernel build/vmlinux $(QEMUOPT)
SRCDIRS=kern lib
SRCS=$(shell find $(SRCDIRS) -name '*.[cS]' -type f)
OBJS=$(SRCS:%=build/%.o)

.PHONY: run
run: build/vmlinux
	@-kill -SIGKILL `ps -a | grep 'qemu.gdb$$' | awk '{print $$1}'` 2> /dev/null || true
	@if echo Ok, GDB | nc -c localhost 1235; then $(QEMUCMD) -S -s; else $(QEMUCMD); fi

build/%.S.o: %.S
	@echo - [as] $<
	@mkdir -p $(@D)
	@nasm -felf -o $@ $<

build/%.c.o: %.c
	@echo - [cc] $<
	@mkdir -p $(@D)
	@gcc -nostdlib -nostdinc -fno-stack-protector $(COPT) -Iinclude -m32 -c -o $@ $<

info:
	@echo COPT=$(COPT)
	@echo QEMUCMD=$(QEMUCMD)
	@echo SRCDIRS=$(SRCDIRS)
	@echo SRCS=$(SRCS)

build/vmlinux: $(OBJS)
	@echo + [ld] $@
	@mkdir -p $(@D)
	@ld -m elf_i386 -e _start -Ttext 0x100000 -o $@ $^ `gcc -m32 -print-libgcc-file-name`
