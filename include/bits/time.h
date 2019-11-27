#ifndef _BITS_TIME_H
#define _BITS_TIME_H 1

#include <sys/types.h>

struct timespec {
	time_t tv_sec;
	suseconds_t tv_nsec;
};

struct itimerspec {
	struct timespec it_period;
	struct timespec it_value;
};

struct timeval {
	time_t tv_sec;
	suseconds_t tv_nsec;
};

struct itimerval {
	struct timeval it_interval;
	struct timeval it_value;
};

struct tm {
	int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
	int tm_min;			/* Minutes.	[0-59] */
	int tm_hour;			/* Hours.	[0-23] */
	int tm_mday;			/* Day.		[1-31] */
	int tm_mon;			/* Month.	[0-11] */
	int tm_year;			/* Year	- 1900.  */
	int tm_wday;			/* Day of week.	[0-6] */
	int tm_yday;			/* Days in year.[0-365]	*/
	int tm_isdst;			/* DST.		[-1/0/1]*/
};

#define CLOCKS_PER_SEC ((clock_t)200)

#endif
