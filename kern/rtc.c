#include <kern/rtc.h>
#include <kern/kernel.h>
#include <kern/asm/cmos.h>

static int
cmos_bcd(int i)
{
	int bcd = cmos_read(0x80 | i);
	return (bcd & 15) + (bcd >> 4) * 10;
}

void
rtc_gettime(struct tm *t)
{
	t->tm_sec = cmos_bcd(0);
	t->tm_min = cmos_bcd(2);
	t->tm_hour = cmos_bcd(4);
	t->tm_mday = cmos_bcd(7);
	t->tm_mon = cmos_bcd(8) - 1;
	t->tm_year = cmos_bcd(9);
}

void
rtc_init(void)
{
	struct tm t;
	rtc_gettime(&t);
	printk("%04d/%02d/%02d %02d:%02d:%02d",
			t.tm_year + 1900, t.tm_mon, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec);
}
