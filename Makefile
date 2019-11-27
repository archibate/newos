DYN=1
DISP=1
#####################################
ifdef RELEASE
OPTIM=3
STRIP=strip
else
COPT+=-ggdb -gstabs+ -D_KDEBUG
endif
ifeq ($(STRIP),)
STRIP=:
endif
COPT+=$(if $(OPTIM), -O$(OPTIM))
CFLAGS+=-m32 -march=i386 -nostdlib -nostdinc $(COPT) \
	-fno-stack-protector -Iinclude -Wall -Wextra \
	-Wno-unused -Wno-main -Wno-frame-address \
	-Wno-builtin-declaration-mismatch \
	-Werror=int-conversion -Werror=implicit-int \
	-Werror=implicit-function-declaration \
	-Wno-format-zero-length -D_NEWOS
ifeq ($(DYN),)
CFLAGS+=-D_LIBC_EXP
endif
ifeq ($(DISP),)
CFLAGS+=-D_TTY_SERIAL
endif
LDFLAGS=-m elf_i386
QEMUOPT=-m 128 -serial stdio $(if $(DISP),,-display none)
QEMUCMD=qemu-system-i386 $(QEMUOPT)
LIBGCC=$(shell gcc $(CFLAGS) -print-libgcc-file-name)
KERN_DIRS=kern mm fs libc/pure
KERN_SRCS=$(shell find $(KERN_DIRS) -name '*.[cS]' -type f)
KERN_OBJS=build/scripts/stext.c.o $(KERN_SRCS:%=build/%.o) build/scripts/ebss.c.o
USER_SRCS=$(shell find usr -name '*.[cS]' -type f)
USER_BINS=$(shell echo $(USER_SRCS:%=build/%) | sed 's/\.[cS]//g')
ifneq ($(DYN),)
USER_LIBS=build/libc.dl
endif
LIBC_SRCS=$(shell find libc -name '*.[cS]' -type f)
LIBC_OBJS=$(LIBC_SRCS:%=build/%.o)
CRT0_OBJS=build/scripts/crt0.c.o

.PHONY: default
default: all

.PHONY: all
all: build/boot.img

.PHONY: run
run: build/boot.img
	@tools/startqemu.sh $(QEMUCMD) -drive file=$<,index=0,media=disk,driver=raw | tee build/qemu.log

.PHONY: bochs
bochs: build/boot.img
	@-bochs -qf tools/bochsrc.bxrc

build/boot.img: build/boot/bootsect.S.bin build/vmlinux.bin build/filesys.txt $(USER_BINS) $(USER_LIBS)
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@rm -f $@ && bximage -q -mode=create -imgmode=flat -hd=10M $@
	@tools/dd.sh if=$< of=$@ bs=2048 count=1 conv=notrunc
	@tools/dd.sh if=$(word 2, $^) of=$@ bs=1024 seek=2 conv=notrunc count=`tools/blks.c $(word 2, $^)`
	@tools/mknefs.c $@ -r `tools/blks.c $(word 2, $^) | awk '{print $$1}'` -L NewOS -f $(word 3, $^)

build/filesys.txt: filesys.txt usr
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@cat $< > $@
	@echo 'bin {' >> $@
	@for x in $(USER_BINS); do echo 0755 `basename $$x` $$x >> $@; done
	@echo '}' >> $@
	@echo 'lib {' >> $@
	@for x in $(USER_LIBS); do echo `basename $$x` $$x >> $@; done
	@echo '}' >> $@

build/boot/bootsect.S.bin: build/boot/kerninfo.inc

build/boot/kerninfo.inc: build/vmlinux build/vmlinux.bin Makefile
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@echo kern_addr equ `readelf -l $< | grep '^\s*LOAD.*R E' | awk '{print $$4; exit}'` > $@
	@echo kern_size equ 1024 \* `tools/blks.c $(word 2, $^)` >> $@
	@echo kmem_size equ $$[`readelf -l build/vmlinux | grep '^\s*LOAD' | tail -n1 | awk '{print $$4" + "$$6}'`] - kern_addr >> $@

build/vmlinux.bin: build/vmlinux
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@objcopy -O binary -S $< $@
	@truncate -s%512 $@

build/%.S.o: %.S
	@echo - '[as]' $<
	@mkdir -p $(@D)
	@nasm -felf -o $@ $<

build/%.S.o.d: %.S
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@nasm -M -MT $(@:%.d=%) -felf $< > $@

build/%.S.bin: %.S
	@echo - '[as]' $<
	@mkdir -p $(@D)
	@nasm -fbin -o $@ $<

build/%.S.bin.d: %.S
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@nasm -M -MT $(@:%.d=%) -fbin $< > $@

build/%.c.o: %.c
	@echo - '[cc]' $<
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -c -o $@ $<

build/%.c.o.d: %.c
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@gcc -M -MT $(@:%.d=%) $(CFLAGS) -c -o $@ $<

ifneq ($(DYN),)
#build/libc/%: CFLAGS+=-fPIC
$(foreach x, $(KERN_DIRS) libc, build/$x/%): CFLAGS+=-D_LIBC_EXP
endif

build/usr/%: build/usr/%.c.o $(CRT0_OBJS) scripts/user.ld build/libc.a
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld -nostdlib $(LDFLAGS) -T scripts/user.ld -L build -e _start -o $@ $(CRT0_OBJS) $< $(LIBGCC) -lc
	@$(STRIP) $@

.PHONY: info
info:
	@echo CFLAGS=$(CFLAGS)
	@echo QEMUCMD=$(QEMUCMD)
	@echo KERN_DIRS=$(KERN_DIRS)
	@echo KERN_SRCS=$(KERN_SRCS)
	@echo LIBC_SRCS=$(LIBC_SRCS)
	@echo USER_SRCS=$(USER_SRCS)
	@echo USER_BINS=$(USER_BINS)
	@echo CRT0_OBJS=$(CRT0_OBJS)
	@echo LDSCRIPT=$(LDSCRIPT)
	@echo LIBGCC=$(LIBGCC)

.PHONY: kernel
kernel: build/vmlinux

build/vmlinux: $(KERN_OBJS)
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld $(LDFLAGS) -static -e _start -Ttext 0x100000 -o $@ $^ $(LIBGCC)
	@$(STRIP) $@

ifeq ($(DYN),)
build/libc.a: $(LIBC_OBJS)
	@echo + '[ar]' $@
	@mkdir -p $(@D)
	@-rm -rf $@
	@ar cqs $@ $^
	@$(STRIP) $@
else
build/libc.dl.nostrip: $(LIBC_OBJS) scripts/dynlib.ld
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld $(LDFLAGS) -static -T scripts/dynlib.ld -o $@ $(LIBC_OBJS)
	@tools/dd.sh of=$@ if=tools/elfdynsig.bin seek=8 count=1 bs=2 conv=notrunc

build/%.dl: build/%.dl.nostrip
ifeq ($(STRIP),:)
	@cp $< $@
else
	@$(STRIP) $< -o $@
endif

build/%.a: build/%.dl.nostrip
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@rm -rf $@
	@tools/makedlo.sh $< $@ /lib/$*.dl
endif

.PHONY: clean
clean:
	rm -rf build

.PHONY: dep
dep: build/dep

build/dep: $(filter %.o.d, $(KERN_OBJS:%=%.d) $(LIBC_OBJS:%=%.d) $(USER_BINS:%=%.d))
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@cat $^ > $@

.PRECIOUS: build/scripts/% build/%.a build/%.dl

include build/dep
