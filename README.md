New OS
======

A hobby unix-like operating system
------

<img src=docs/1.png>

### Equipped with:

- a simple shell.
- filesystem **NeFS** made from stratch.
- busybox-like command line utils.
- **vi** as the text editor.
- **fork()** copy-on-write.
- ELF dynamic linking like a DLL.
- Simple C standard library.
- I/O multiplex by **ionotify()**,
  a subset of poll() from stratch.
- optional graphical user interface
  (toggle by `VIDEO=1` in Makefile).
- simple window manager based on **System V** IPC.
- simple **IDL** langurage generating IPC code.

### Enviroument:

- Arch Linux (x86_64)
- gcc 9.1.0
- nasm 2.14.02
- binutils 2.32 (for ld, strip, objcopy)
- coreutils 8.31 (for cat, dd, etc.)
- make 4.2.1 (for Makefile)
- qemu-system-i386 4.1.0 (for simulation)
- bochs 2.6.9 (for bximage)
- python 3.8.0 (for tools/idl.py)
- gdb 8.3.1 (for debugging)
- tigervnc 1.13.1 (for viewing)

#### To build:
```bash
make all
```

will create `build/boot.img`.

#### To run:
```bash
make run
```

will run our OS in qemu.

`vncviewer :5900`

to view the console.

try typing this in our console:
```bash
vi /usr/src/snake.c
```

#### To run in graphics:
```bash
make clean
make run VIDEO=1
```

will run our OS in graphics mode (work in progress).

```bash
vncviewer :5900
```

to view the graphics.

try typing this in the serial terminal:
```bash
bkg xinitd
xtest
```

### Errors on your machine?
Please contact me at <u>17721388340@163.com</u>.
