#include <sys/io.h>
#include <string.h>
#include <kern/tty.h>
#include <ctype.h>

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
	static int state, num, last_num;
	int count, start;
	c &= 0xff;

	if (!state)
		num = last_num = 0;

	if (state == 2) {
		switch (c) {
		case '?':
		case '[':
			state = 3;
			return 1;
		case 'A':
			vga_pos -= VGA_COLS;
			break;
		case 'B':
			vga_pos += VGA_COLS;
			break;
		case 'C':
			vga_pos--;
			break;
		case 'D':
			vga_pos++;
			break;
		case 'K':
			start = vga_pos;
			count = 0;
			switch (num) {
			case 0:
				count = VGA_COLS - vga_pos % VGA_COLS;
				break;
			case 1:
				start -= vga_pos % VGA_COLS;
				count = (vga_pos + VGA_COLS - 1) % VGA_COLS + 1;
				break;
			case 2:
			case 3:
				start -= vga_pos % VGA_COLS;
				count = VGA_COLS;
				break;
			}
fill:
			while (count--) {
				vga_buf[start++] = vga_color;
			}
			break;
		case 'J':
			start = 0;
			count = 0;
			switch (num) {
			case 0:
				start = vga_pos;
				count = VGA_SIZE - vga_pos;
				break;
			case 1:
				count = vga_pos;
				break;
			case 2:
			case 3:
				count = VGA_SIZE;
				break;
			}
			goto fill;
		case 'm':
			if (last_num) {
				last_num ^= num;
				num ^= last_num;
				last_num ^= num;
			}
csim:
			switch (num) {
			case 0:
				vga_color = 0x0700;
				break;
			case 1:
				vga_color |= 0x0800;
				break;
			case 7:
				vga_color = ((vga_color & 0xf000) >> 4)
					  | ((vga_color & 0x0f00) << 4);
				break;
			}
			if (last_num) {
				num = last_num;
				last_num = 0;
				goto csim;
			}
			break;
		case 'H':
			if (!last_num) {
				last_num = num;
				num = 1;
			}
			if (!last_num) last_num = 1;
			vga_pos = (last_num - 1) * VGA_COLS + (num - 1);
			break;
		case 'h':
		case 'l':
			/* Hide / Show not supported for now */
			break;
		case ';':
			last_num = num;
			num = 0;
			return 1;
		default:
			if (isdigit(c)) {
				num = num * 10 + (c - '0');
				return 1;
			}
		};

	} else if (state == 1) {
		switch (c) {
		case '[':
			state = 2;
			return 1;
		};

	} else {
		switch (c) {
		case '\b':
			if (vga_pos > 0)
				vga_buf[--vga_pos] = vga_color | ' ';
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
				vga_buf[vga_pos++] = vga_color | ' ';
			break;
		case 27:
			state = 1;
			return 1;
		default:
			vga_buf[vga_pos++] = vga_color | c;
		}
	}

	state = 0;

	while (vga_pos < 0)
		vga_pos += VGA_COLS;
	if (vga_pos >= VGA_SIZE) {
		memmove(vga_buf, vga_buf + VGA_COLS,
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

	extern int kbd_getc(int *c);
	tty_register(TTY_VGA, vga_putc, kbd_getc);
}
