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
