DISP=1
#VIDEO=1
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
	-Wno-unused -Wno-frame-address -Waddress-of-packed-member \
	-Wno-builtin-declaration-mismatch -Werror=return-type \
	-Werror=int-conversion -Werror=return-local-addr \
	-Werror=implicit-function-declaration -Werror=implicit-int \
	-Werror=discarded-qualifiers -Wno-format-zero-length \
	-Werror=incompatible-pointer-types -Wno-main \
	-D_NEWOS
ifneq ($(VIDEO),)
CFLAGS+=-D_TTY_SERIAL -D_VIDEO
NASMFLAGS+=-D_VIDEO
else
ifeq ($(DISP),)
CFLAGS+=-D_TTY_SERIAL
endif
endif
LDFLAGS=-m elf_i386
QEMUOPT=-m 128 -serial stdio $(if $(DISP),,-display none)
QEMUCMD=qemu-system-i386 $(QEMUOPT)
LIBGCC=$(shell gcc $(CFLAGS) -print-libgcc-file-name)
KERN_DIRS=kern mm fs libc/pure
KERN_SRCS=$(shell find $(KERN_DIRS) -name '*.[cS]' -type f)
KERN_OBJS=build/scripts/stext.c.o $(KERN_SRCS:%=build/%.o) build/scripts/ebss.c.o
USER_SRCS=$(shell find usr -name '*.[cS]' -type f)
USER_LIBS=c
ifneq ($(VIDEO),)
USER_LIBS+=rax
endif
CRT0_OBJS=build/scripts/crt0.c.o
SRCS=$(shell find -L . -name '*.[cS]' -type f | sed 's/\.\///' | uniq -u | grep -v '\(trash\|tools\|include\/idl\)')

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

build/boot.img: build/boot/bootsect.S.bin build/vmlinux.bin build/usr/busybox build/filesys.txt rcs.txt $(USER_LIBS:%=build/lib%.dl)
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@-rm -rf $@
	@bximage -q -mode=create -imgmode=flat -hd=10M $@
	@tools/dd.sh if=$< of=$@ bs=2048 count=1 conv=notrunc
	@tools/dd.sh if=$(word 2, $^) of=$@ bs=1024 seek=2 conv=notrunc count=`tools/blks.c $(word 2, $^)`
	@tools/mknefs.c $@ -r `tools/blks.c build/vmlinux.bin | awk '{print $$1}'` -L NewOS -f build/filesys.txt

build/filesys.txt: filesys.txt usr
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@cat $< > $@
	@echo 'bin {' >> $@
	@echo 0755 busybox build/usr/busybox >> $@
	@for x in $(USER_SRCS:%.c=%); do echo `basename $$x` '->' busybox >> $@; done
	@echo '}' >> $@
	@echo 'lib {' >> $@
	@for x in $(USER_LIBS); do echo lib$$x.dl build/lib$$x.dl >> $@; done
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
	@nasm -felf $(NASMFLAGS) -o $@ $<

build/%.S.o.d: %.S
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@nasm -M -MT $(@:%.d=%) -felf $(NASMFLAGS) $< > $@

build/%.S.bin: %.S
	@echo - '[as]' $<
	@mkdir -p $(@D)
	@nasm -fbin $(NASMFLAGS) -o $@ $<

build/%.S.bin.d: %.S
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@nasm -M -MT $(@:%.d=%) $(NASMFLAGS) -fbin $< > $@

build/%.c.o: %.c
	@echo - '[cc]' $<
	@mkdir -p $(@D)
	@gcc $(CFLAGS) -c -o $@ $<

build/%.c.o.d: %.c
	@echo - '[dep]' $<
	@mkdir -p $(@D)
	@gcc -M -MT $(@:%.d=%) $(CFLAGS) -c -o $@ $<

build/vmlinux: $(KERN_OBJS)
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld $(LDFLAGS) -static -e _start -Ttext 0x100000 -o $@ $^ $(LIBGCC)
	@$(STRIP) $@

build/%.dl.nostrip build/%.a build/usr/%: SRCS=$(shell find $* -name '*.[cS]' -type f)

include/idl/%.c include/idl/%.h include/idl/%.svr.c: scripts/%.idl tools/idl.py
	@echo - '[idl]' $<
	@tools/idl.py $< include/idl/$*.c include/idl/$*.h include/idl/$*.svr.c

#build/lib%: CFLAGS+=-fPIC
$(foreach x, $(KERN_DIRS) libc, build/$x/%): CFLAGS+=-D_LIBC_EXP

build/librax.%: LIBS+=c

build/usr/busybox: $(USER_SRCS:%=build/%.o) $(CRT0_OBJS) build/libc.a scripts/user.ld
	@make $(USER_LIBS:%=build/lib%.a)
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld -nostdlib $(LDFLAGS) -T scripts/user.ld -L build -o $@ $(CRT0_OBJS) $(USER_SRCS:%=build/%.o) $(LIBGCC) $(USER_LIBS:%=-l%)
	@$(STRIP) $@

build/%.dl.nostrip: scripts/%.ld
	@make $(SRCS:%=build/%.o) $(LIBS:%=build/lib%.a)
	@echo + '[ld]' $@
	@mkdir -p $(@D)
	@ld $(LDFLAGS) -static -T scripts/$*.ld -L build -o $@ $(SRCS:%=build/%.o) $(LIBS:%=-l%)
	@tools/dd.sh of=$@ if=tools/elfdynsig.bin seek=8 count=1 bs=2 conv=notrunc

build/%.dl: build/%.dl.nostrip
	@echo + '[gen]' $@
ifeq ($(STRIP),:)
	@cp $< $@
else
	@$(STRIP) $< -o $@
endif

build/%.a: build/%.dl.nostrip
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@-rm -rf $@
	@tools/makedlo.sh $< $@ /lib/$*.dl

.PHONY: clean
clean:
	-rm -rf build dep

dep: $(SRCS:%=build/%.o.d)
	@echo + '[gen]' $@
	@mkdir -p $(@D)
	@cat $^ > $@

.PRECIOUS: %.d build/scripts/% build/%.a build/%.dl

-include dep
