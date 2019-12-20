#ifdef _VIDEO
#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/notify.h>
#include <signal.h>
#include <rax/bits.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define MAP_VRAM 1

// Part of ASC16: {{{
const int asc16[1024] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x817e0000, 0xbd8181a5, 
	0x7e818199, 0x00000000, 0xff7e0000, 0xc3ffffdb, 0x7effffe7, 0x00000000, 
	0x00000000, 0xfefefe6c, 0x10387cfe, 0x00000000, 0x00000000, 0xfe7c3810, 
	0x0010387c, 0x00000000, 0x18000000, 0xe7e73c3c, 0x3c1818e7, 0x00000000, 
	0x18000000, 0xffff7e3c, 0x3c18187e, 0x00000000, 0x00000000, 0x3c180000, 
	0x0000183c, 0x00000000, 0xffffffff, 0xc3e7ffff, 0xffffe7c3, 0xffffffff, 
	0x00000000, 0x42663c00, 0x003c6642, 0x00000000, 0xffffffff, 0xbd99c3ff, 
	0xffc399bd, 0xffffffff, 0x0e1e0000, 0xcc78321a, 0x78cccccc, 0x00000000, 
	0x663c0000, 0x3c666666, 0x18187e18, 0x00000000, 0x333f0000, 0x3030303f, 
	0xe0f07030, 0x00000000, 0x637f0000, 0x6363637f, 0xe6e76763, 0x000000c0, 
	0x18000000, 0xe73cdb18, 0x1818db3c, 0x00000000, 0xe0c08000, 0xf8fef8f0, 
	0x80c0e0f0, 0x00000000, 0x0e060200, 0x3efe3e1e, 0x02060e1e, 0x00000000, 
	0x3c180000, 0x1818187e, 0x00183c7e, 0x00000000, 0x66660000, 0x66666666, 
	0x66660066, 0x00000000, 0xdb7f0000, 0x1b7bdbdb, 0x1b1b1b1b, 0x00000000, 
	0x60c67c00, 0xc6c66c38, 0xc60c386c, 0x0000007c, 0x00000000, 0x00000000, 
	0xfefefefe, 0x00000000, 0x3c180000, 0x1818187e, 0x7e183c7e, 0x00000000, 
	0x3c180000, 0x1818187e, 0x18181818, 0x00000000, 0x18180000, 0x18181818, 
	0x183c7e18, 0x00000000, 0x00000000, 0xfe0c1800, 0x0000180c, 0x00000000, 
	0x00000000, 0xfe603000, 0x00003060, 0x00000000, 0x00000000, 0xc0c00000, 
	0x0000fec0, 0x00000000, 0x00000000, 0xfe6c2800, 0x0000286c, 0x00000000, 
	0x00000000, 0x7c383810, 0x00fefe7c, 0x00000000, 0x00000000, 0x7c7cfefe, 
	0x00103838, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	0x3c180000, 0x18183c3c, 0x18180018, 0x00000000, 0x66666600, 0x00000024, 
	0x00000000, 0x00000000, 0x6c000000, 0x6c6cfe6c, 0x6c6cfe6c, 0x00000000, 
	0xc67c1818, 0x067cc0c2, 0x7cc68606, 0x00001818, 0x00000000, 0x180cc6c2, 
	0x86c66030, 0x00000000, 0x6c380000, 0xdc76386c, 0x76cccccc, 0x00000000, 
	0x30303000, 0x00000060, 0x00000000, 0x00000000, 0x180c0000, 0x30303030, 
	0x0c183030, 0x00000000, 0x18300000, 0x0c0c0c0c, 0x30180c0c, 0x00000000, 
	0x00000000, 0xff3c6600, 0x0000663c, 0x00000000, 0x00000000, 0x7e181800, 
	0x00001818, 0x00000000, 0x00000000, 0x00000000, 0x18181800, 0x00000030, 
	0x00000000, 0xfe000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
	0x18180000, 0x00000000, 0x00000000, 0x180c0602, 0x80c06030, 0x00000000, 
	0x6c380000, 0xd6d6c6c6, 0x386cc6c6, 0x00000000, 0x38180000, 0x18181878, 
	0x7e181818, 0x00000000, 0xc67c0000, 0x30180c06, 0xfec6c060, 0x00000000, 
	0xc67c0000, 0x063c0606, 0x7cc60606, 0x00000000, 0x1c0c0000, 0xfecc6c3c, 
	0x1e0c0c0c, 0x00000000, 0xc0fe0000, 0x06fcc0c0, 0x7cc60606, 0x00000000, 
	0x60380000, 0xc6fcc0c0, 0x7cc6c6c6, 0x00000000, 0xc6fe0000, 0x180c0606, 
	0x30303030, 0x00000000, 0xc67c0000, 0xc67cc6c6, 0x7cc6c6c6, 0x00000000, 
	0xc67c0000, 0x067ec6c6, 0x780c0606, 0x00000000, 0x00000000, 0x00001818, 
	0x00181800, 0x00000000, 0x00000000, 0x00001818, 0x30181800, 0x00000000, 
	0x06000000, 0x6030180c, 0x060c1830, 0x00000000, 0x00000000, 0x00007e00, 
	0x0000007e, 0x00000000, 0x60000000, 0x060c1830, 0x6030180c, 0x00000000, 
	0xc67c0000, 0x18180cc6, 0x18180018, 0x00000000, 0x7c000000, 0xdedec6c6, 
	0x7cc0dcde, 0x00000000, 0x38100000, 0xfec6c66c, 0xc6c6c6c6, 0x00000000, 
	0x66fc0000, 0x667c6666, 0xfc666666, 0x00000000, 0x663c0000, 0xc0c0c0c2, 
	0x3c66c2c0, 0x00000000, 0x6cf80000, 0x66666666, 0xf86c6666, 0x00000000, 
	0x66fe0000, 0x68786862, 0xfe666260, 0x00000000, 0x66fe0000, 0x68786862, 
	0xf0606060, 0x00000000, 0x663c0000, 0xdec0c0c2, 0x3a66c6c6, 0x00000000, 
	0xc6c60000, 0xc6fec6c6, 0xc6c6c6c6, 0x00000000, 0x183c0000, 0x18181818, 
	0x3c181818, 0x00000000, 0x0c1e0000, 0x0c0c0c0c, 0x78cccccc, 0x00000000, 
	0x66e60000, 0x78786c66, 0xe666666c, 0x00000000, 0x60f00000, 0x60606060, 
	0xfe666260, 0x00000000, 0xeec60000, 0xc6d6fefe, 0xc6c6c6c6, 0x00000000, 
	0xe6c60000, 0xcedefef6, 0xc6c6c6c6, 0x00000000, 0xc67c0000, 0xc6c6c6c6, 
	0x7cc6c6c6, 0x00000000, 0x66fc0000, 0x607c6666, 0xf0606060, 0x00000000, 
	0xc67c0000, 0xc6c6c6c6, 0x7cded6c6, 0x00000e0c, 0x66fc0000, 0x6c7c6666, 
	0xe6666666, 0x00000000, 0xc67c0000, 0x0c3860c6, 0x7cc6c606, 0x00000000, 
	0x7e7e0000, 0x1818185a, 0x3c181818, 0x00000000, 0xc6c60000, 0xc6c6c6c6, 
	0x7cc6c6c6, 0x00000000, 0xc6c60000, 0xc6c6c6c6, 0x10386cc6, 0x00000000, 
	0xc6c60000, 0xd6d6c6c6, 0x6ceefed6, 0x00000000, 0xc6c60000, 0x38387c6c, 
	0xc6c66c7c, 0x00000000, 0x66660000, 0x183c6666, 0x3c181818, 0x00000000, 
	0xc6fe0000, 0x30180c86, 0xfec6c260, 0x00000000, 0x303c0000, 0x30303030, 
	0x3c303030, 0x00000000, 0x80000000, 0x3870e0c0, 0x02060e1c, 0x00000000, 
	0x0c3c0000, 0x0c0c0c0c, 0x3c0c0c0c, 0x00000000, 0xc66c3810, 0x00000000, 
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000ff00, 
	0x00183030, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x7c0c7800, 
	0x76cccccc, 0x00000000, 0x60e00000, 0x666c7860, 0x7c666666, 0x00000000, 
	0x00000000, 0xc0c67c00, 0x7cc6c0c0, 0x00000000, 0x0c1c0000, 0xcc6c3c0c, 
	0x76cccccc, 0x00000000, 0x00000000, 0xfec67c00, 0x7cc6c0c0, 0x00000000, 
	0x6c380000, 0x60f06064, 0xf0606060, 0x00000000, 0x00000000, 0xcccc7600, 
	0x7ccccccc, 0x0078cc0c, 0x60e00000, 0x66766c60, 0xe6666666, 0x00000000, 
	0x18180000, 0x18183800, 0x3c181818, 0x00000000, 0x06060000, 0x06060e00, 
	0x06060606, 0x003c6666, 0x60e00000, 0x786c6660, 0xe6666c78, 0x00000000, 
	0x18380000, 0x18181818, 0x3c181818, 0x00000000, 0x00000000, 0xd6feec00, 
	0xc6d6d6d6, 0x00000000, 0x00000000, 0x6666dc00, 0x66666666, 0x00000000, 
	0x00000000, 0xc6c67c00, 0x7cc6c6c6, 0x00000000, 0x00000000, 0x6666dc00, 
	0x7c666666, 0x00f06060, 0x00000000, 0xcccc7600, 0x7ccccccc, 0x001e0c0c, 
	0x00000000, 0x6676dc00, 0xf0606060, 0x00000000, 0x00000000, 0x60c67c00, 
	0x7cc60c38, 0x00000000, 0x30100000, 0x3030fc30, 0x1c363030, 0x00000000, 
	0x00000000, 0xcccccc00, 0x76cccccc, 0x00000000, 0x00000000, 0x66666600, 
	0x183c6666, 0x00000000, 0x00000000, 0xd6c6c600, 0x6cfed6d6, 0x00000000, 
	0x00000000, 0x386cc600, 0xc66c3838, 0x00000000, 0x00000000, 0xc6c6c600, 
	0x7ec6c6c6, 0x00f80c06, 0x00000000, 0x18ccfe00, 0xfec66030, 0x00000000, 
	0x180e0000, 0x18701818, 0x0e181818, 0x00000000, 0x18180000, 0x18001818, 
	0x18181818, 0x00000000, 0x18700000, 0x180e1818, 0x70181818, 0x00000000, 
	0xdc760000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc66c3810, 
	0x00fec6c6, 0x00000000, 0x663c0000, 0xc0c0c0c2, 0x0c3c66c2, 0x00007c06, 
	0x00cc0000, 0xcccccc00, 0x76cccccc, 0x00000000, 0x30180c00, 0xfec67c00, 
	0x7cc6c0c0, 0x00000000, 0x6c381000, 0x7c0c7800, 0x76cccccc, 0x00000000, 
	0x00cc0000, 0x7c0c7800, 0x76cccccc, 0x00000000, 0x18306000, 0x7c0c7800, 
	0x76cccccc, 0x00000000, 0x386c3800, 0x7c0c7800, 0x76cccccc, 0x00000000, 
	0x00000000, 0x6060663c, 0x060c3c66, 0x0000003c, 0x6c381000, 0xfec67c00, 
	0x7cc6c0c0, 0x00000000, 0x00c60000, 0xfec67c00, 0x7cc6c0c0, 0x00000000, 
	0x18306000, 0xfec67c00, 0x7cc6c0c0, 0x00000000, 0x00660000, 0x18183800, 
	0x3c181818, 0x00000000, 0x663c1800, 0x18183800, 0x3c181818, 0x00000000, 
	0x18306000, 0x18183800, 0x3c181818, 0x00000000, 0x1000c600, 0xc6c66c38, 
	0xc6c6c6fe, 0x00000000, 0x00386c38, 0xc6c66c38, 0xc6c6c6fe, 0x00000000, 
	0x00603018, 0x7c6066fe, 0xfe666060, 0x00000000, 0x00000000, 0x3676cc00, 
	0x6ed8d87e, 0x00000000, 0x6c3e0000, 0xccfecccc, 0xcecccccc, 0x00000000, 
	0x6c381000, 0xc6c67c00, 0x7cc6c6c6, 0x00000000, 0x00c60000, 0xc6c67c00, 
	0x7cc6c6c6, 0x00000000, 0x18306000, 0xc6c67c00, 0x7cc6c6c6, 0x00000000, 
	0xcc783000, 0xcccccc00, 0x76cccccc, 0x00000000, 0x18306000, 0xcccccc00, 
	0x76cccccc, 0x00000000, 0x00c60000, 0xc6c6c600, 0x7ec6c6c6, 0x00780c06, 
	0x7c00c600, 0xc6c6c6c6, 0x7cc6c6c6, 0x00000000, 0xc600c600, 0xc6c6c6c6, 
	0x7cc6c6c6, 0x00000000, 0x3c181800, 0x60606066, 0x18183c66, 0x00000000, 
	0x646c3800, 0x6060f060, 0xfce66060, 0x00000000, 0x66660000, 0x187e183c, 
	0x1818187e, 0x00000000, 0xccccf800, 0xdeccc4f8, 0xc6cccccc, 0x00000000, 
	0x181b0e00, 0x187e1818, 0x18181818, 0x000070d8, 0x60301800, 0x7c0c7800, 
	0x76cccccc, 0x00000000, 0x30180c00, 0x18183800, 0x3c181818, 0x00000000, 
	0x60301800, 0xc6c67c00, 0x7cc6c6c6, 0x00000000, 0x60301800, 0xcccccc00, 
	0x76cccccc, 0x00000000, 0xdc760000, 0x6666dc00, 0x66666666, 0x00000000, 
	0xc600dc76, 0xdefef6e6, 0xc6c6c6ce, 0x00000000, 0x6c6c3c00, 0x007e003e, 
	0x00000000, 0x00000000, 0x6c6c3800, 0x007c0038, 0x00000000, 0x00000000, 
	0x30300000, 0x60303000, 0x7cc6c6c0, 0x00000000, 0x00000000, 0xc0fe0000, 
	0x00c0c0c0, 0x00000000, 0x00000000, 0x06fe0000, 0x00060606, 0x00000000, 
	0xc2c0c000, 0x3018ccc6, 0x0c86dc60, 0x00003e18, 0xc2c0c000, 0x3018ccc6, 
	0x3e9ece66, 0x00000606, 0x18180000, 0x18181800, 0x183c3c3c, 0x00000000, 
	0x00000000, 0xd86c3600, 0x0000366c, 0x00000000, 0x00000000, 0x366cd800, 
	0x0000d86c, 0x00000000, 0x44114411, 0x44114411, 0x44114411, 0x44114411, 
	0xaa55aa55, 0xaa55aa55, 0xaa55aa55, 0xaa55aa55, 0x77dd77dd, 0x77dd77dd, 
	0x77dd77dd, 0x77dd77dd, 0x18181818, 0x18181818, 0x18181818, 0x18181818, 
	0x18181818, 0xf8181818, 0x18181818, 0x18181818, 0x18181818, 0xf818f818, 
	0x18181818, 0x18181818, 0x36363636, 0xf6363636, 0x36363636, 0x36363636, 
	0x00000000, 0xfe000000, 0x36363636, 0x36363636, 0x00000000, 0xf818f800, 
	0x18181818, 0x18181818, 0x36363636, 0xf606f636, 0x36363636, 0x36363636, 
	0x36363636, 0x36363636, 0x36363636, 0x36363636, 0x00000000, 0xf606fe00, 
	0x36363636, 0x36363636, 0x36363636, 0xfe06f636, 0x00000000, 0x00000000, 
	0x36363636, 0xfe363636, 0x00000000, 0x00000000, 0x18181818, 0xf818f818, 
	0x00000000, 0x00000000, 0x00000000, 0xf8000000, 0x18181818, 0x18181818, 
	0x18181818, 0x1f181818, 0x00000000, 0x00000000, 0x18181818, 0xff181818, 
	0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x18181818, 0x18181818, 
	0x18181818, 0x1f181818, 0x18181818, 0x18181818, 0x00000000, 0xff000000, 
	0x00000000, 0x00000000, 0x18181818, 0xff181818, 0x18181818, 0x18181818, 
	0x18181818, 0x1f181f18, 0x18181818, 0x18181818, 0x36363636, 0x37363636, 
	0x36363636, 0x36363636, 0x36363636, 0x3f303736, 0x00000000, 0x00000000, 
	0x00000000, 0x37303f00, 0x36363636, 0x36363636, 0x36363636, 0xff00f736, 
	0x00000000, 0x00000000, 0x00000000, 0xf700ff00, 0x36363636, 0x36363636, 
	0x36363636, 0x37303736, 0x36363636, 0x36363636, 0x00000000, 0xff00ff00, 
	0x00000000, 0x00000000, 0x36363636, 0xf700f736, 0x36363636, 0x36363636, 
	0x18181818, 0xff00ff18, 0x00000000, 0x00000000, 0x36363636, 0xff363636, 
	0x00000000, 0x00000000, 0x00000000, 0xff00ff00, 0x18181818, 0x18181818, 
	0x00000000, 0xff000000, 0x36363636, 0x36363636, 0x36363636, 0x3f363636, 
	0x00000000, 0x00000000, 0x18181818, 0x1f181f18, 0x00000000, 0x00000000, 
	0x00000000, 0x1f181f00, 0x18181818, 0x18181818, 0x00000000, 0x3f000000, 
	0x36363636, 0x36363636, 0x36363636, 0xff363636, 0x36363636, 0x36363636, 
	0x18181818, 0xff18ff18, 0x18181818, 0x18181818, 0x18181818, 0xf8181818, 
	0x00000000, 0x00000000, 0x00000000, 0x1f000000, 0x18181818, 0x18181818, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xff000000, 
	0xffffffff, 0xffffffff, 0xf0f0f0f0, 0xf0f0f0f0, 0xf0f0f0f0, 0xf0f0f0f0, 
	0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0xffffffff, 0x00ffffff, 
	0x00000000, 0x00000000, 0x00000000, 0xd8dc7600, 0x76dcd8d8, 0x00000000, 
	0xcc780000, 0xccd8cccc, 0xccc6c6c6, 0x00000000, 0xc6fe0000, 0xc0c0c0c6, 
	0xc0c0c0c0, 0x00000000, 0x00000000, 0x6c6c6cfe, 0x6c6c6c6c, 0x00000000, 
	0xfe000000, 0x183060c6, 0xfec66030, 0x00000000, 0x00000000, 0xd8d87e00, 
	0x70d8d8d8, 0x00000000, 0x00000000, 0x66666666, 0x60607c66, 0x000000c0, 
	0x00000000, 0x1818dc76, 0x18181818, 0x00000000, 0x7e000000, 0x66663c18, 
	0x7e183c66, 0x00000000, 0x38000000, 0xfec6c66c, 0x386cc6c6, 0x00000000, 
	0x6c380000, 0x6cc6c6c6, 0xee6c6c6c, 0x00000000, 0x301e0000, 0x663e0c18, 
	0x3c666666, 0x00000000, 0x00000000, 0xdbdb7e00, 0x00007edb, 0x00000000, 
	0x03000000, 0xdbdb7e06, 0xc0607ef3, 0x00000000, 0x301c0000, 0x607c6060, 
	0x1c306060, 0x00000000, 0x7c000000, 0xc6c6c6c6, 0xc6c6c6c6, 0x00000000, 
	0x00000000, 0xfe0000fe, 0x00fe0000, 0x00000000, 0x00000000, 0x187e1818, 
	0xff000018, 0x00000000, 0x30000000, 0x0c060c18, 0x7e003018, 0x00000000, 
	0x0c000000, 0x30603018, 0x7e000c18, 0x00000000, 0x1b0e0000, 0x1818181b, 
	0x18181818, 0x18181818, 0x18181818, 0x18181818, 0x70d8d8d8, 0x00000000, 
	0x00000000, 0x7e001818, 0x00181800, 0x00000000, 0x00000000, 0x00dc7600, 
	0x0000dc76, 0x00000000, 0x6c6c3800, 0x00000038, 0x00000000, 0x00000000, 
	0x00000000, 0x18000000, 0x00000018, 0x00000000, 0x00000000, 0x00000000, 
	0x00000018, 0x00000000, 0x0c0c0f00, 0xec0c0c0c, 0x1c3c6c6c, 0x00000000, 
	0x6c6cd800, 0x006c6c6c, 0x00000000, 0x00000000, 0x30d87000, 0x00f8c860, 
	0x00000000, 0x00000000, 0x00000000, 0x7c7c7c7c, 0x007c7c7c, 0x00000000, 
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 
};
// }}}
// Part of Handle: {{{
enum handle_type {
	HT_None = 0,
	HT_DC, HT_Window, HT_Listener,
};

struct handle {
	struct handle *next, **pprev;
	int hint;
	int type;
};

#define HASH_MAX 23
#define HASH(x) ((x) * 257 % 23)

static struct handle *handles[HASH_MAX];

static void *GetHandle(int hint, int type)
{
	int i = HASH(hint);
	struct handle *h;
	for (h = handles[i]; h; h = h->next) {
		if (h->hint == hint) {
			if (!type || h->type == type)
				return h;
			else
				break;
		}
	}
	return NULL;
}

static int now_hint_top = 1;

static void *CreateHandle(int type, size_t size)
{
	int hint = now_hint_top++;
	int i = HASH(hint);
	struct handle *h = calloc(size, 1);
	h->hint = hint;
	h->type = type;
	h->next = handles[i];
	if (handles[i])
		handles[i]->pprev = &h->next;
	h->pprev = &handles[i];
	handles[i] = h;
	return h;
}

static void DestroyHandle(void *hp)
{
	struct handle *h = hp;
	if (h->next)
		h->next->pprev = h->pprev;
	*h->pprev = h->next;
	h->pprev = (void *)-1;
	h->next = (void *)-1;
	free(h);
}

// }}}
// Part of Buffer: {{{

typedef struct RGB {
	char r, g, b;
} __attribute__((packed)) RGB;

typedef union RGBA {
	struct {
		char r, g, b, a;
	} __attribute__((packed));
	struct RGB rgb;
	int w;
} __attribute__((packed)) RGBA;

#define rgb2w(rgb) (*(int *)&(rgb) & 0xffffff)
#define w2rgb(_w) (((RGBA) {.w = (_w)}).rgb)
#define rgb3(_r, _g, _b) ((RGB) {.r = (_r), .g = (_g), .b = (_b)})
#define rgb1(_u) rgb3(((_u) >> 16) & 0xff, ((_u) >> 8) & 0xff, (_u) & 0xff)

struct buf {
	RGB *rgb;
	int sx, sy;
	int nx, ny;
	int ismybuf;
	RGBA col_inv;
};

static void buf_create(struct buf *b, int nx, int ny)
{
	b->rgb = calloc(nx * sizeof(RGB), ny);
	b->sx = 1;
	b->sy = nx;
	b->nx = nx;
	b->ny = ny;
	b->ismybuf = 1;
}

static void buf_destroy(struct buf *b)
{
	if (b->ismybuf)
		free(b->rgb);
}

static void buf_pointSanity(struct buf *b, int *x, int *y)
{
	if (*x < 0)
		*x = 0;
	if (*y < 0)
		*y = 0;
	if (*x > b->nx)
		*x = b->nx;
	if (*y > b->ny)
		*y = b->ny;
}

static void buf_rectSanity(struct buf *b,
		int *x0, int *y0, int *x1, int *y1)
{
	int nx = *x1 - *x0;
	int ny = *y1 - *y0;
	if (nx < 0) {
		*x0 += nx;
		*x1 -= nx;
	}
	if (ny < 0) {
		*y0 += ny;
		*y1 -= ny;
	}
	buf_pointSanity(b, x0, y0);
	buf_pointSanity(b, x1, y1);
}

static void buf_subRect(struct buf *b, struct buf *d,
		int x0, int y0, int x1, int y1)
{
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	d->rgb = b->rgb + x0 * b->sx + y0 * b->sy;
	d->sx = b->sx;
	d->sy = b->sy;
	d->nx = x1 - x0;
	d->ny = y1 - y0;
	d->ismybuf = 0;
}

static void buf_blitSubSub(struct buf *b, struct buf *d,
		int x0, int y0, int sx0, int sy0, int sx1, int sy1)
{
	RGB *p, *q;
	int x, y, xo = x0 - sx0, yo = y0 - sy0;
	int x1 = x0 + sx1 - sx0, y1 = y0 + sy1 - sy0;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	//printf("blit %p <- %p: %d %d %d %d\n", b, d, x0, y0, x1, y1);
	if (!d->col_inv.a) {
		for (y = y0; y < y1; y++) {
			for (x = x0; x < x1; x++) {
				p = b->rgb + x * b->sx + y * b->sy;
				q = d->rgb + (x - xo) * d->sx + (y - yo) * d->sy;
				*p = *q;
			}
		}
	} else {
		int col_inv = rgb2w(d->col_inv.rgb);
		for (y = y0; y < y1; y++) {
			for (x = x0; x < x1; x++) {
				p = b->rgb + x * b->sx + y * b->sy;
				q = d->rgb + (x - xo) * d->sx + (y - yo) * d->sy;
				if (rgb2w(*q) != col_inv)
					*p = *q;
			}
		}
	}
}
static void buf_blitSub(struct buf *b, struct buf *d,
		int x0, int y0)
{
	buf_blitSubSub(b, d, x0, y0, 0, 0, d->nx, d->ny);
}

static void buf_fillRect(struct buf *b,
		int x0, int y0, int x1, int y1,
		RGB color)
{
	RGB *p;
	int x, y;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			p = b->rgb + x * b->sx + y * b->sy;
			*p = color;
		}
	}
}

static void buf_setPixel(struct buf *b, int x, int y, RGB color)
{
	buf_fillRect(b, x, y, x + 1, y + 1, color);
}

static void buf_textOut(struct buf *b,
		int x0, int y0, const char *s, int count,
		RGB color)
{
	int x1 = x0 + count * 8, y1 = y0 + 16;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	for (int x = x0; x < x1; x += 8) {
		int c = *s++;
		if (!isascii(c)) c = '?';
		const char *a = (const char *)asc16 + c * 16;
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 8; j++) {
				if (a[i] & (1 << (7 - j)))
					buf_setPixel(b, x + j, y0 + i, color);
			}
		}
	}
}

// }}}
// Part of Screen: {{{

static int g_nx, g_ny, g_bpp;
static size_t g_vram_size;
static void *g_vram;

static void screen_init(void)
{
	int g_screen_f = open("/dev/fb0", O_RDWR);
	if (g_screen_f == -1) {
		perror("/dev/fb0");
		exit(1);
	}
	g_nx = ioctl(g_screen_f, I_FB_GET_NX);
	g_ny = ioctl(g_screen_f, I_FB_GET_NY);
	g_bpp = ioctl(g_screen_f, I_FB_GET_BPP);
	if (g_nx == -1 || g_ny == -1 || g_bpp == -1) {
		perror("cannot ioctl /dev/fb0");
		exit(1);
	}
	g_vram_size = g_nx * g_ny * (g_bpp == 24 ? 3 : 1);
#ifdef MAP_VRAM
	g_vram = (void *)0x70000000;
	g_vram = mmap(g_vram, g_vram_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, g_screen_f, 0);
	printf("g_vram at %p\n", g_vram);
#else
	g_vram = malloc(g_vram_size);
#endif
	if (g_vram == MAP_FAILED) {
		perror("cannot mmap /dev/fb0");
		exit(1);
	}
	close(g_screen_f);
}

// }}}
// Part of Listener: {{{

static int g_msq_m;

struct Listener
{
	struct handle h;
};

static void ListenerCallback(struct Listener *l, struct Message *msg)
{
	msg->hlst = l->h.hint;
	msgsnd(g_msq_m, msg, sizeof(*msg) - sizeof(msg->hwnd),
			//IPC_NOWAIT | MSG_NOERROR);
			MSG_NOERROR);
}

static void do_XCreateListener(int *hlst)
{
	struct Listener *l = CreateHandle(HT_Listener, sizeof(struct Listener));
	*hlst = l->h.hint;
}

static void do_XDestroyListener(int hlst)
{
	struct Listener *l = GetHandle(hlst, HT_Listener);
	if (!l) return;
	DestroyHandle(l);
}

// }}}
// Part of Window: {{{

#define WMTOP 18

struct Window {
	struct handle h;
	struct buf b, *dcb;
	int x0, y0, zindex;
	int cx0, cy0, cx1, cy1;
	struct Window *parent, *children;
	struct Window *next, **pprev;
	struct Listener *listener;
	int flags;
	int isdown;
	char *text;
};

struct Window *g_desktop, *g_act_win;

static void UpdateScreen(void)
{
#ifdef MAP_VRAM
	//if (g_bpp == 24 && g_desktop->b.rgb != g_vram) memcpy(g_vram, g_desktop->b.rgb, g_vram_size);
	if (g_bpp != 24) {
		for (int y = 0; y < g_ny; y++) {
			for (int x = 0; x < g_nx; x++) {
				char *q = g_vram + y * g_nx + x;
				RGB *p = g_desktop->b.rgb
					+ y * g_desktop->b.sy
					+ x * g_desktop->b.sx;
#if 0
				int c = (((p->r * 3) >> 8) & 0x03)
				      |	(((p->g * 6) >> 6) & 0x1c)
				      |	(((p->b * 6) >> 3) & 0xe0);
#else
				int c = (((p->b * 3) >> 8) & 0x03)
				      |	(((p->g * 6) >> 6) & 0x1c)
				      |	(((p->r * 6) >> 3) & 0xe0);
#endif
				*q = c;
			}
		}
	}
	if (-1 == msync(g_vram, g_vram_size, MS_SYNC))
		perror("cannot msync video memory");
#else
	FILE *g_screen_f = fopen("/dev/fb0", "w");
	if (!g_screen_f) {
		perror("cannot open /dev/fb0");
		return;
	}
	if (g_bpp == 24) {
		if (fwrite(g_vram, g_nx * g_ny * 3,
					1, g_screen_f) != 1)
			perror("cannot update video buffer");
	} else {
		for (int y = 0; y < g_ny; y++) {
			for (int x = 0; x < g_nx; x++) {
				RGB *p = g_vram + y * g_nx + x;
				int c = (((p->b * 3) >> 8) & 0x03)
				      |	(((p->g * 6) >> 6) & 0x1c)
				      |	(((p->r * 6) >> 3) & 0xe0);
				putc(c, g_screen_f);
			}
		}
	}
	fclose(g_screen_f);
#endif
}
#if 0
#define RG /2+
#define GB *4+32*
#else
#define RG *255/6+
#define GB *255/6*255+255*255*255/6*
#endif

static void ListenerBind(struct Listener *l, struct Window *w, int deep)
{
	w->listener = l;
	if (deep) {
		struct Window *u;
		for (u = w->children; u; u = u->next)
			ListenerBind(l, u, deep);
	}
}

static void draw_label(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, rgb1(0x808080));
	if (w->text)
		buf_textOut(b, 0,//b->nx / 2 - 4 * strlen(w->text),
				b->ny / 2 - 8, w->text,
				strlen(w->text), rgb1(0));
}

static void draw_button_up(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, rgb1(0xc0c0c0));
	buf_fillRect(b, 0, b->ny - 2, b->nx, b->ny, rgb1(0x808080));
	buf_fillRect(b, b->nx - 2, 0, b->nx, b->ny, rgb1(0x808080));
	buf_fillRect(b, 0, 0, b->nx, 1, rgb1(0xf0f0f0));
	buf_fillRect(b, 0, 0, 1, b->ny, rgb1(0xf0f0f0));
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, rgb1(0));
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, rgb1(0));
	if (w->text)
		buf_textOut(b, b->nx / 2 - 4 * strlen(w->text),
				b->ny / 2 - 8, w->text,
				strlen(w->text), rgb1(0));
}

static void draw_button_down(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, rgb1(0x808080));
	buf_fillRect(b, 0, 0, b->nx, 2, rgb1(0x404040));
	buf_fillRect(b, 0, 0, 2, b->ny, rgb1(0x404040));
	buf_fillRect(b, 0, 0, b->nx, 1, rgb1(0));
	buf_fillRect(b, 0, 0, 1, b->ny, rgb1(0));
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, rgb1(0xc0c0c0));
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, rgb1(0xc0c0c0));
	if (w->text)
		buf_textOut(b, b->nx / 2 - 4 * strlen(w->text) + 1,
				b->ny / 2 - 7, w->text,
				strlen(w->text), rgb1(0));
}

static void draw_caption(struct Window *w)
{
	int i;
	struct buf b0, *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, rgb1(0x808080));
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, rgb1(0x404040));
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, rgb1(0x404040));
	buf_fillRect(b, 0, 0, b->nx, 1, rgb1(0xc0c0c0));
	buf_fillRect(b, 0, 0, 1, b->ny, rgb1(0xc0c0c0));
	buf_fillRect(b, 1, 1, b->nx - 1, WMTOP - 1, rgb1(0xa00000));
	if (w->text)
		buf_textOut(b, 4, 2, w->text,
				strlen(w->text), rgb1(0xc0c0c0));

	b = &b0;
	buf_subRect(&w->b, b, w->b.nx - WMTOP, 0, w->b.nx, WMTOP);
	buf_fillRect(b, 3, 3, WMTOP - 3, WMTOP - 3, rgb1(0xc0c0c0));
	buf_fillRect(b, 3, WMTOP - 5, WMTOP - 3, WMTOP - 3, rgb1(0x808080));
	buf_fillRect(b, WMTOP - 5, WMTOP - 3, WMTOP - 3, 3, rgb1(0x808080));
	buf_fillRect(b, 3, 3, WMTOP - 3, 4, rgb1(0xf0f0f0));
	buf_fillRect(b, 3, WMTOP - 3, 4, 3, rgb1(0xf0f0f0));
	buf_fillRect(b, 3, WMTOP - 4, WMTOP - 3, WMTOP - 3, rgb1(0));
	buf_fillRect(b, WMTOP - 4, WMTOP - 3, WMTOP - 3, 3, rgb1(0));

	for (i = 6; i < WMTOP - 6; i++)
		buf_setPixel(b, i, WMTOP - 1 - i, rgb1(0x404040));
	for (i = 6; i < WMTOP - 6; i++)
		buf_setPixel(b, i, i - 1, rgb1(0));
	for (i = 5; i < WMTOP - 7; i++)
		buf_setPixel(b, i, i + 1, rgb1(0));
	for (i = 5; i < WMTOP - 7; i++)
		buf_setPixel(b, i, WMTOP - 3 - i, rgb1(0));
	for (i = 5; i < WMTOP - 6; i++)
		buf_setPixel(b, i, i, rgb1(0));
	for (i = 5; i < WMTOP - 6; i++)
		buf_setPixel(b, i, WMTOP - 2 - i, rgb1(0));
	buf_setPixel(b, WMTOP - 7, WMTOP - 7, rgb1(0x404040));
}

static void SetWindowText(struct Window *w, const char *text)
{
	if (text) {
		if (w->text)
			free(w->text);
		w->text = strndup(text, 32);
	}

	if ((w->flags & 0xff) == WT_CAPTION) {
		draw_caption(w);

	} else if ((w->flags & 0xff) == WT_BUTTON) {
		if (w->isdown)
			draw_button_down(w);
		else
			draw_button_up(w);

	} else if ((w->flags & 0xff) == WT_LABEL) {
		draw_label(w);
	}
}

static struct Window *CreateWindow(struct Window *parent,
		int x0, int y0, int nx, int ny, int zindex, int flags)
{
	struct Window *w, **pp;
	w = CreateHandle(HT_Window, sizeof(struct Window));

	if ((flags & 0xff) == WT_CAPTION) {
		nx += 2;
		ny += WMTOP;
	}

	if (!(flags & WF_NOSEL))
		g_act_win = w;

	w->parent = parent;
	if (parent) {
		pp = &parent->children;
		while (*pp) {
			if ((*pp)->zindex >= zindex)
				break;
			pp = &(*pp)->next;
		}
		if ((w->next = (*pp)))
			w->next->pprev = &w->next;
		w->pprev = pp;
		*pp = w;
	}
	w->x0 = x0;
	w->y0 = y0;
	w->flags = flags;
	w->zindex = zindex;
	buf_create(&w->b, nx, ny);

	w->cx0 = w->cy0 = 0;
	w->cx1 = w->b.nx;
	w->cy1 = w->b.ny;
	if ((w->flags & 0xff) == WT_CAPTION) {
		w->cx0 = 1;
		w->cy0 = WMTOP - 1;
		w->cx1 = w->b.nx - 1;
		w->cy1 = w->b.ny - 1;
	}
	SetWindowText(w, NULL);
	return w;
}

static struct Window *JustUpdateWindow(struct Window *w)
{
	struct buf pcb;
	struct Window *parent;
	for (; (parent = w->parent); w = parent) {
		buf_subRect(&parent->b, &pcb,
				parent->cx0, parent->cy0,
				parent->cx1, parent->cy1);
		do {
			buf_blitSub(&pcb, &w->b, w->x0, w->y0);
		} while ((w = w->next));
	}
	return w;
}

static void UpdateWindow(struct Window *w)
{
	w = JustUpdateWindow(w);
	if (w == g_desktop)
		UpdateScreen();
}

#if 0
static int HasWindowIntersection(struct Window *u, struct Window *v)
{
}
#endif

static void EraseWindow(struct Window *w)
{
	struct buf pcb;
	struct Window *u, *parent = w->parent;
	buf_subRect(&parent->b, &pcb,
			parent->cx0, parent->cy0,
			parent->cx1, parent->cy1);
	if (parent->dcb) {
		buf_blitSubSub(&pcb, parent->dcb,
				w->x0, w->y0, w->x0, w->y0,
				w->x0 + w->b.nx, w->y0 + w->b.ny);
	} else {
		buf_fillRect(&pcb, w->x0, w->y0,
				w->x0 + w->b.nx, w->y0 + w->b.ny, rgb1(0));
	}
	for (u = parent->children; u && u != w; u = u->next) {
		// if (HasWindowIntersection(u, v))
		buf_blitSub(&parent->b, &u->b, u->x0, u->y0);
	}
}

static void RefreshWindow(struct Window *w, int deep)
{
	struct Window *u;
	struct buf pcb;
	SetWindowText(w, NULL);
	buf_subRect(&w->b, &pcb, w->cx0, w->cy0, w->cx1, w->cy1);
	for (u = w->children; u; u = u->next) {
		if (deep) RefreshWindow(u, deep);
		buf_blitSub(&pcb, &u->b, u->x0, u->y0);
	}
}

static void DestroyWindow(struct Window *w)
{
	EraseWindow(w);
	UpdateWindow(w->parent);
	if (w->text) free(w->text);
	buf_destroy(&w->b);
	if (w->next)
		w->next->pprev = w->pprev;
	*w->pprev = w->next;
	DestroyHandle(w);
}

static void SendMessage(struct Window *w, struct Message *msg)
{
	if (!w->listener) printf("window without listener %d\n", w->h.hint);
	if (!w->listener)
		return;
	msg->hwnd = w->h.hint;
	ListenerCallback(w->listener, msg);
}

static void on_window_lbutton(struct Window *w, int isdown, int x, int y)
{
	if (!(w->flags & WF_CLICK))
		return;
	struct Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = isdown ? WM_MOUSE_LDOWN : WM_MOUSE_LUP;
	msg.pos.x = x;
	msg.pos.y = y;
	w->isdown = isdown;
	RefreshWindow(w, 0);
	UpdateWindow(w);
	SendMessage(w, &msg);
}

static void SetWindowPos(struct Window *w, int x0, int y0)
{
	EraseWindow(w);
	w->x0 = x0;
	w->y0 = y0;
	UpdateWindow(w);
}

static int IsPointInWindowRect(struct Window *w, int x, int y)
{
	return x >= w->x0 && y >= w->y0 &&
		x < w->x0 + w->b.nx && y < w->y0 + w->b.ny;
}

static struct Window *FindWindowUnder(struct Window *w,
		int *px, int *py, struct Window **mov)
{
	int x = *px, y = *py;
	x -= w->cx0; y -= w->cy0;
	struct Window *u, *r = NULL;
	for (u = w->children; u; u = u->next) {
		if (u->flags & WF_NOSEL)
			continue;
		if (!IsPointInWindowRect(u, x, y))
			continue;
		r = u;
	}
	if (r) {
		x -= r->x0; y -= r->y0;
		if (r->flags & WF_MOVE)
			*mov = r;
		r = FindWindowUnder(r, &x, &y, mov);
	}
	if (!r && !(w->flags & WF_NOSEL))
		r = w;
	if (r) {
		*px = x;
		*py = y;
	}
	return r;
}

static void do_XCreateWindow(int *hwnd, int hparent,
		int x0, int y0, int nx, int ny, int flags)
{
	struct Window *w, *parent = g_desktop;
	if (hparent) {
		parent = GetHandle(hparent, HT_Window);
		if (!parent) return;
	}
	w = CreateWindow(parent, x0, y0,
			nx, ny, 1, flags);
	*hwnd = w->h.hint;
}

static void do_XSetWindowPos(int hwnd, int x0, int y0)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	SetWindowPos(w, x0, y0);
}

static void do_XSetWindowText(int hwnd, const char *text)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	SetWindowText(w, text);
}

static void do_XDestroyWindow(int hint)
{
	struct Window *w = GetHandle(hint, HT_Window);
	if (!w) return;
	DestroyWindow(w);
}

static void do_XUpdateWindow(int hwnd)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	UpdateWindow(w);
}

static void do_XRefreshWindow(int hwnd, int deep)
{
	struct Window *w = g_desktop;
	if (hwnd) {
		w = GetHandle(hwnd, HT_Window);
		if (!w) return;
	}
	RefreshWindow(w, deep);
}

static void do_XListenerBind(int hlst, int hwnd, int deep)
{
	struct Listener *l = GetHandle(hlst, HT_Listener);
	if (!l) return;
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	ListenerBind(l, w, deep);
}

static void desktop_init(void)
{
	g_desktop = CreateWindow(NULL, 0, 0, g_nx, g_ny, 0, WF_NOSEL);
#ifndef MAP_VRAM
	g_vram = g_desktop->b.rgb;
#else
	if (g_bpp == 24)
		g_desktop->b.rgb = g_vram;
#endif
}

// }}}
// Part of DC: {{{

struct DC {
	struct handle h;
	struct buf b;
	struct Window *window;
	RGBA bgcolor, fgcolor;
};

static struct DC *CreateDC(struct Window *w)
{
	if (w->dcb)
		return NULL;
	struct DC *dc = CreateHandle(HT_DC, sizeof(struct DC));
	//buf_subRect(&w->b, &dc->b, w->cx0, w->cy0, w->cx1, w->cy1);
	dc->window = w;
	buf_create(&dc->b, w->cx1 - w->cx0, w->cy1 - w->cy0);
	dc->bgcolor.rgb = rgb1(0);
	dc->fgcolor.rgb = rgb1(0xc0c0c0);
	w->dcb = &dc->b;
	return dc;
}

static void UpdateDC(struct DC *dc)
{
	struct Window *w = dc->window;
	buf_blitSub(&w->b, &dc->b, w->cx0, w->cy0);
}

static void DestroyDC(struct DC *dc)
{
	buf_destroy(&dc->b);
	dc->window->dcb = NULL;
	DestroyHandle(dc);
}

static void do_XCreateDC(int *hdc, int hwnd)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	struct DC *dc = CreateDC(w);
	if (!dc) return;
	*hdc = dc->h.hint;
}

static void do_XUpdateDC(int hdc)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	UpdateDC(dc);
}

static void do_XDestroyDC(int hint)
{
	struct DC *dc = GetHandle(hint, HT_DC);
	if (!dc) return;
	DestroyDC(dc);
}

static void do_XSetFillStyle(int hdc, int bgcolor, int fgcolor)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	dc->fgcolor.rgb = rgb1(fgcolor);
	dc->bgcolor.rgb = rgb1(bgcolor);
}

static void do_XFillRect(int hdc, int x0, int y0, int x1, int y1)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	buf_fillRect(&dc->b, x0, y0, x1, y1, dc->bgcolor.rgb);
}

static void do_XSetPixel(int hdc, int x0, int y0)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	buf_setPixel(&dc->b, x0, y0, dc->fgcolor.rgb);
}

static void do_XTextOut(int hdc, int x0, int y0,
		const char *text, int count)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	buf_textOut(&dc->b, x0, y0, text, count, dc->fgcolor.rgb);
}

// }}}
// Part of Mouse: {{{

static struct Window *g_mouse;
static int g_mouse_fd;
#define LMB 1
#define MMB 2
#define RMB 4
static int g_mouse_button;
static int g_mx, g_my, g_sel_x, g_sel_y;
static struct Window *g_sel_win, *g_mov_win;

static void do_poll(int sig, sigset_t blk, long arg);

static void on_mouse_lbutton(int isdown)
{
	if (isdown) {
		g_mov_win = NULL;
		g_sel_x = g_mx;
		g_sel_y = g_my;
		g_sel_win = FindWindowUnder(g_desktop,
				&g_sel_x, &g_sel_y, &g_mov_win);
		if (g_sel_win)
			g_act_win = g_sel_win;
	}
	if (g_sel_win) {
		on_window_lbutton(g_sel_win, isdown, g_sel_x, g_sel_y);
	}
	if (!isdown) {
		g_sel_win = NULL;
		g_mov_win = NULL;
	}
}
 
static void on_mouse_move(int dx, int dy)
{
	static int wx, wy;
	if (g_mov_win) {
		SetWindowPos(g_mov_win,
			g_mov_win->x0 + dx,
			g_mov_win->y0 + dy);
	}
	SetWindowPos(g_mouse, g_mx, g_my);
}

#if 0
void show_mouse_info(void)
{
	printf("mouse %c%c%c %4d %4d\n",
			"L-"[!(g_mouse_button & LMB)],
			"M-"[!(g_mouse_button & MMB)],
			"R-"[!(g_mouse_button & RMB)],
			g_mx, g_my);
}
#endif

static void on_mouse_button_change(int button)
{
	int omb = g_mouse_button;
	g_mouse_button = button & 0x7;
	omb ^= g_mouse_button;
	if (omb & LMB)
		on_mouse_lbutton(g_mouse_button & LMB);
}

static void on_mouse_pos_change(int dx, int dy)
{
	int omx = g_mx, omy = g_my;
	g_mx += dx; g_my += dy;
	buf_pointSanity(&g_desktop->b, &g_mx, &g_my);
	dx = g_mx - omx;
	dy = g_my - omy;
	if (!dx && !dy)
		return;

	on_mouse_move(dx, dy);
}

static void do_mouse(void)
{
	char data[3];
	while (1) {
		data[0] = data[1] = data[2] = 0;
		if (read(g_mouse_fd, data, 1) != 1)
			break;
		if (data[0] != 27)
			continue;
		if (read(g_mouse_fd, data, 3) != 3)
			break;
		on_mouse_button_change(data[0]);
		if (data[1] || data[2])
			on_mouse_pos_change(data[1], data[2]);
	}
}

static void mouse_init(void)
{
	static char cursor[16][8] = {
		"**......",
		"*O*.....",
		"*O*.....",
		"*OO*....",
		"*OO*....",
		"*OOO*...",
		"*OOO*...",
		"*OOOO*..",
		"*OOOO*..",
		"*OOOOO*.",
		"*OOOOO*.",
		"*OOOOOO*",
		"*OOO****",
		"*O**....",
		"**......",
		"........",
	};

	g_mouse_fd = open("/dev/mouse", O_RDONLY | O_NONBLOCK);
	if (g_mouse_fd == -1) {
		perror("/dev/mouse");
		exit(1);
	}

	ioctl(g_mouse_fd, I_CLBUF);

	g_mx = g_nx / 2;
	g_my = g_ny / 2;

	g_mouse = CreateWindow(g_desktop, g_mx, g_my,
			8, 16, 256, WF_NOSEL);
	g_mouse->b.col_inv.rgb = rgb1(0xdeadea);
	g_mouse->b.col_inv.a = 1;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			RGB c;
			switch (cursor[i][j]) {
			case 'O': c = rgb1(0xc0c0c0); break;
			case '*': c = rgb1(0); break;
			default: c = g_mouse->b.col_inv.rgb; break;
			};
			buf_setPixel(&g_mouse->b, j, i, c);
		}
	}
	UpdateWindow(g_mouse);
}

// }}}
// Part of Keyboard: {{{
 
static int g_keybd_fd;

static void on_window_keybd(struct Window *w, int isdown, int key)
{
	if (!(w->flags & WF_KEYDOWN))
		return;
	struct Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = isdown ? WM_KEYBD_DOWN : WM_KEYBD_UP;
	msg.key = key;
	SendMessage(w, &msg);
}

static void on_keybd(int isdown, int key)
{
	if (g_act_win) {
		on_window_keybd(g_act_win, isdown, key);
	}
}

static void do_keybd(void)
{
	char data[1];
	while (1) {
		if (read(g_keybd_fd, data, 1) != 1)
			break;
		//printf("kbd got [%c]\n", data[0]);
		on_keybd(1, data[0]);
	}
}

struct termios tc_orig, tc_raw;

static void cookmode(void)
{
	tcsetattr(g_keybd_fd, TCSANOW, &tc_orig);
}

static void keybd_init(void)
{
	g_keybd_fd = open("/dev/keybd", O_RDONLY | O_NONBLOCK);
	if (g_keybd_fd == -1) {
		perror("/dev/keybd");
		exit(1);
	}

	ioctl(g_keybd_fd, I_CLBUF);
	if (-1 != tcgetattr(g_keybd_fd, &tc_orig)) {
		memcpy(&tc_raw, &tc_orig, sizeof(struct termios));
		tc_raw.c_lflag &= ~(ICANON | ECHO | ISIG);
		if (-1 == tcsetattr(g_keybd_fd, TCSANOW, &tc_raw))
			perror("cannot tcsetattr /dev/keybd");
		else
			atexit(cookmode);
	}
}

// }}}
// Part of IPC: {{{

static int g_msq, g_msq_r;

static void ipc_init(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1) {
		perror("/dev/fb0");
		exit(1);
	}
	key_t key_r = ftok("/dev/fb0", 6666);
	key_t key_m = ftok("/dev/fb0", 8888);

	g_msq = msgget(key, IPC_CREAT | IPC_EXCL);
	if (g_msq == -1) {
		perror("cannot create msqueue for X server");
		exit(1);
	}
	g_msq_r = msgget(key_r, IPC_CREAT | IPC_EXCL);
	if (g_msq_r == -1) {
		perror("cannot create replying msqueue for X server");
		exit(1);
	}
	g_msq_m = msgget(key_m, IPC_CREAT | IPC_EXCL);
	if (g_msq_m == -1) {
		perror("cannot create transfering msqueue for X server");
		exit(1);
	}
}

// }}}

static void do_poll(int sig, sigset_t blk, long arg)
{
	ssetmask(-1);
	signal(SIGPOLL, (void *)do_poll);
	ionotify(g_mouse_fd, ION_READ, 1);
	ionotify(g_keybd_fd, ION_READ, 2);
	do_keybd();
	do_mouse();
	ssetmask(0);
}

int main(int argc, char **argv)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("cannot fork X server to background");
		return 1;
	}
	if (pid > 0) {
		printf("X-server started in background, pid=%d\n", pid);
		return 0;
	}

	stdout = stderr;

	ipc_init();
	screen_init();
	desktop_init();
	mouse_init();
	keybd_init();
	signal(SIGPOLL, (void *)do_poll);
	if (-1 == ionotify(g_keybd_fd, ION_READ, 2))
		perror("cannot ionotify /dev/keybd");
	if (-1 == ionotify(g_mouse_fd, ION_READ, 1))
		perror("cannot ionotify /dev/mouse");

	while (1) {
#include <idl/rax.svr.c>
	}
	return 0;
}
#endif
