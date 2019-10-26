COPT=-ggdb -gstabs+
QEMUOPT=-m 128 -serial stdio $(if $(DISP),,-display none)
QEMUCMD=qemu-system-i386 $(QEMUOPT)
SRCDIRS=kern mm fs lib
SRCS=$(shell find $(SRCDIRS) -name '*.[cS]' -type f)
OBJS+=build/tools/stext.c.o
OBJS+=$(SRCS:%=build/%.o)
OBJS+=$(shell gcc -m32 -print-libgcc-file-name)
OBJS+=build/tools/ebss.c.o

.PHONY: default
default: boot

.PHONY: image-test
image-test: build/boot.img
	@hexdump -C $< | less

.PHONY: run
run: build/vmlinux
	@tools/startqemu.sh $(QEMUCMD) -kernel $<

.PHONY: boot
boot: build/boot.img
	@tools/startqemu.sh $(QEMUCMD) -drive file=$<,index=0,media=disk,driver=raw

.PHONY: bochs
bochs: build/boot.img
	@-bochs -qf tools/bochsrc.bxrc

build/boot.img: build/boot/bootsect.S.bin build/vmlinux.bin
	@echo + [gen] $@
	@mkdir -p $(@D)
	@cat $^ > $@
	@tools/mknefs.c $@ -r $$[1 + `du $(word 2, $^) | awk '{print $$1}'`]

build/vmlinux.bin: build/vmlinux
	@echo + [gen] $@
	@mkdir -p $(@D)
	@objcopy -O binary -S $< $@

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
	@gcc -m32 -nostdlib -nostdinc -fno-stack-protector $(COPT) -D_KERNEL -Iinclude -c -o $@ $<

.PHONY: info
info:
	@echo COPT=$(COPT)
	@echo QEMUCMD=$(QEMUCMD)
	@echo SRCDIRS=$(SRCDIRS)
	@echo SRCS=$(SRCS)

build/vmlinux: $(OBJS)
	@echo + [ld] $@
	@mkdir -p $(@D)
	@ld -m elf_i386 -e _start -Ttext 0x100000 -o $@ $^
