#include <stdlib.h>
#include <unistd.h>

div_t div(int a, int b)
{
	div_t d;
	d.quot = a / b;
	d.rem = a % b;
	return d;
}

int abs(int x)
{
	return x < 0 ? -x : x;
}

int atoi(const char *s)
{
	return strtol(s, NULL, 10);
}

int rand_r(unsigned int *sp)
{
	*sp = *sp * 1103515245 + 12345;
	return (unsigned)(*sp >> 16) % (RAND_MAX + 1);

}

static unsigned int seed = 1;

void srand(unsigned int s)
{
	seed = s;
}

int rand(void)
{
	return rand_r(&seed);
}
