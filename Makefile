COPT=-ggdb -gstabs+ $(if $(OPTIM), -O$(OPTIM))
CFLAGS=-m32 -march=i386 -nostdlib -nostdinc $(COPT) \
	-fno-stack-protector -Iinclude
QEMUOPT=-m 128 -serial stdio $(if $(DISP),,-display none)
QEMUCMD=qemu-system-i386 $(QEMUOPT)
SRCDIRS=kern mm fs lib
SRCS=$(shell find $(SRCDIRS) -name '*.[cS]' -type f)
OBJS=build/tools/stext.c.o
OBJS+=$(SRCS:%=build/%.o)
OBJS+=$(shell gcc -m32 -print-libgcc-file-name)
OBJS+=build/tools/ebss.c.o

.PHONY: default
default: run

.PHONY: image-test
image-test: build/boot.img
	@hexdump -C $< | less

.PHONY: run
run: build/boot.img
	@tools/startqemu.sh $(QEMUCMD) -drive file=$<,index=0,media=disk,driver=raw

.PHONY: bochs
bochs: build/boot.img
	@-bochs -qf tools/bochsrc.bxrc

build/boot.img: build/boot/bootsect.S.bin build/vmlinux.bin tools/file_list.txt
	@echo + [gen] $@
	@mkdir -p $(@D)
	@cat $^ > $@
	@tools/mknefs.c $@ -r $$[1 + `du $(word 2, $^) | awk '{print $$1}'`] \
		-L NewOS -f $(word 3, $^)

build/boot/bootsect.S.bin: build/boot/kerninfo.inc

build/boot/kerninfo.inc: build/vmlinux build/vmlinux.bin Makefile
	@echo + [gen] $@
	@mkdir -p $(@D)
	@echo kern_addr equ `readelf -l $< | grep '^\s*LOAD.*R E' | awk '{print $$4; exit}'` > $@
	@echo kern_size equ 1024 \* $$[(1 + `du $(word 2, $^) | awk '{print $$1}'`)] >> $@
	@echo kmem_size equ $$[`readelf -l $< | grep '^\s*LOAD' | awk '{print $$6, "+"}'` 0] >> $@

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

.PHONY: info
info:
	@echo CFLAGS=$(CFLAGS)
	@echo QEMUCMD=$(QEMUCMD)
	@echo SRCDIRS=$(SRCDIRS)
	@echo SRCS=$(SRCS)

build/vmlinux: $(OBJS)
	@echo + [ld] $@
	@mkdir -p $(@D)
	@ld -m elf_i386 -e _start -Ttext 0x100000 -o $@ $^
