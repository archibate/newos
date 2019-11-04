COPT=-ggdb -gstabs+ $(if $(OPTIM), -O$(OPTIM))
CFLAGS=-m32 -march=i386 -nostdlib -nostdinc $(COPT) \
	-fno-stack-protector -Iinclude -Wall -Wextra \
	-Wno-unused -Wno-main -Wno-frame-address \
	-Wno-builtin-declaration-mismatch \
	-Werror=int-conversion -Werror=implicit-int \
	-Werror=implicit-function-declaration
QEMUOPT=-m 128 -serial stdio $(if $(DISP),,-display none)
QEMUCMD=qemu-system-i386 $(QEMUOPT)
KERN_DIRS=kern mm fs libc/pure
KERN_SRCS=$(shell find $(KERN_DIRS) -name '*.[cS]' -type f)
KERN_OBJS=build/tools/stext.c.o $(KERN_SRCS:%=build/%.o) \
	 $(shell gcc -m32 -print-libgcc-file-name) build/tools/ebss.c.o
USER_SRCS=$(shell find usr -name '*.c' -type f)
USER_BINS=$(USER_SRCS:%.c=build/%)
LIBC_SRCS=$(shell find libc -name '*.[cS]' -type f)
LIBC_OBJS=$(LIBC_SRCS:%=build/%.o)

.PHONY: default
default: run

.PHONY: image-dump
image-dump: build/boot.img
	@hexdump -C $< | less

.PHONY: run
run: build/boot.img
	@tools/startqemu.sh $(QEMUCMD) -drive file=$<,index=0,media=disk,driver=raw | tee build/qemu.log

.PHONY: bochs
bochs: build/boot.img
	@-bochs -qf tools/bochsrc.bxrc

build/boot.img: build/boot/bootsect.S.bin build/vmlinux.bin filesys.txt $(USER_BINS)
	@echo + [gen] $@
	@mkdir -p $(@D)
	@rm -f $@ && bximage -q -mode=create -imgmode=flat -hd=10M $@
	@dd if=$< of=$@ bs=2048 count=1 conv=notrunc
	@dd if=$(word 2, $^) of=$@ bs=1024 seek=2 conv=notrunc count=`tools/blks.c $(word 2, $^)`
	@tools/mknefs.c $@ -r `tools/blks.c $(word 2, $^) | awk '{print $$1}'` -L NewOS -f $(word 3, $^)

build/boot/bootsect.S.bin: build/boot/kerninfo.inc

build/boot/kerninfo.inc: build/vmlinux build/vmlinux.bin Makefile
	@echo + [gen] $@
	@mkdir -p $(@D)
	@echo kern_addr equ `readelf -l $< | grep '^\s*LOAD.*R E' | awk '{print $$4; exit}'` > $@
	@echo kern_size equ 1024 \* `tools/blks.c $(word 2, $^)` >> $@
	@echo kmem_size equ $$[`readelf -l build/vmlinux | grep '^\s*LOAD' | tail -n1 | awk '{print $$4" + "$$6}'`] - kern_addr >> $@

build/vmlinux.bin: build/vmlinux
	@echo + [gen] $@
	@mkdir -p $(@D)
	@objcopy -O binary -S $< $@
	@tools/fixsects.c $@

build/%.S.o: %.S
	@echo - [as] $<
	@mkdir -p $(@D)
	@nasm -felf -o $@ $<

build/%.S.bin: %.S
	@echo - [as] $<
	@mkdir -p $(@D)
	@nasm -fbin -o $@ $<

build/%.c.o: %.c
	@echo - [cc] $<
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -D_KERNEL -c -o $@ $<

build/usr/%: build/usr/%.c.o build/libc.a
	@ld -m elf_i386 -static -e _start -Ttext 0x40000000 -o $@ $^

.PHONY: info
info:
	@echo CFLAGS=$(CFLAGS)
	@echo QEMUCMD=$(QEMUCMD)
	@echo KERN_DIRS=$(KERN_DIRS)
	@echo KERN_SRCS=$(KERN_SRCS)
	@echo LIBC_SRCS=$(LIBC_SRCS)
	@echo USER_SRCS=$(USER_SRCS)

build/vmlinux: $(KERN_OBJS)
	@echo + [ld] $@
	@mkdir -p $(@D)
	@ld -m elf_i386 -static -e _start -Ttext 0x100000 -o $@ $^

build/libc.a: $(LIBC_OBJS)
	@echo + [ar] $@
	@mkdir -p $(@D)
	@ar cqs $@ $^
