#include <sys/io.h>
#include <string.h>
#include <kern/tty.h>

/*** Text-mode CGA/VGA display output ***/

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_SIZE (VGA_COLS * VGA_ROWS)

static int vga_port;
static int vga_pos;
static int vga_color = 0x0700;
static unsigned short *vga_buf;

static void
vga_download_cursor(void)
{
	outb(vga_port, 14);
	vga_pos = inb(vga_port+1) << 8;
	outb(vga_port, 15);
	vga_pos |= inb(vga_port+1);
}

static void
vga_update_cursor(void)
{
	outb(vga_port, 14);
	outb(vga_port+1, vga_pos >> 8);
	outb(vga_port, 15);
	outb(vga_port+1, vga_pos & 0xff);
}

static int
vga_putc(int c)
{
	if (!(c & ~0xff))
		c |= vga_color;

	switch (c & 0xff) {
	case '\b':
		if (vga_pos > 0)
			vga_buf[--vga_pos] = (c & 0xff) | ' ';
		break;
	case '\n':
		vga_pos += VGA_COLS;
		__attribute__((fallthrough));
	case '\r':
		vga_pos -= vga_pos % VGA_COLS;
		break;
	case '\t':
		for (int end = (vga_pos + 1) % VGA_COLS;
				vga_pos < end; vga_pos++)
			vga_buf[vga_pos++] = c;
		break;
	default:
		vga_buf[vga_pos++] = c;
		break;
	}

	if (vga_pos >= VGA_SIZE) {
		memcpy(vga_buf, vga_buf + VGA_COLS,
				(VGA_SIZE - VGA_COLS) * sizeof(short));
		for (int i = VGA_SIZE - VGA_COLS; i < VGA_SIZE; i++)
			vga_buf[i] = 0x0700 | ' ';
		vga_pos -= VGA_COLS;
	}
	vga_update_cursor();
	return 1;
}

void
vga_init(void)
{
	volatile unsigned short *p = (volatile void *) 0xb8000;
	unsigned short old = *p;
	// test if CGA buffer accessible
	*p ^= 0xa55a;
	if (*p != (old ^ 0xa55a)) {
		// Mono display
		vga_buf = (void *) 0xb0000;
		vga_port = 0x3b4;
	} else {
		// CGA display
		*p = old;
		vga_port = 0x3d4;
		vga_buf = (void *) p;
	}
	vga_download_cursor();

	tty_register(TTY_VGA, vga_putc, NULL);
}
