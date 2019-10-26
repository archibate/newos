#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
const char message[] =
"==========================================\n"
"Welcome to snake.c  (C)  archibate  2019\n\n"
"Use your keyboard aswd, hjkl or arrow keys\n"
"to control the direction where snake goes.\n\n"
"Your target is to eat more food '*', which\n"
"can make your snake body longer like this:\n"
"          ###@ * --> ####@\n\n"
"Functional keys:\n"
"   ESC - quit       SPACE - pause\n"
"     i - speed up       o - slow down\n"
"==========================================\n"
;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <pthread.h>

#define _sleep(x) usleep((x)*1000)

char kbbuf[256];
unsigned char kbwr, kbrd;
pthread_t conio_pid;
pthread_mutex_t conio_mutex = PTHREAD_MUTEX_INITIALIZER;

int kbhit(void)
{
	pthread_mutex_lock(&conio_mutex);
	int ret = kbwr != kbrd;
	pthread_mutex_unlock(&conio_mutex);
	return ret;
}

void *conio_routine(void *unused)
{
	int c;
	while (1) {
		c = getchar();
		pthread_mutex_lock(&conio_mutex);
		if (kbrd - kbwr != 1)
			kbbuf[kbwr++] = c;
		pthread_mutex_unlock(&conio_mutex);
	}
}

int getch(void)
{
	pthread_mutex_lock(&conio_mutex);
	while (kbwr == kbrd) {
		pthread_mutex_unlock(&conio_mutex);
		usleep(10000); // too bad, use semaphore instead
		pthread_mutex_lock(&conio_mutex);
	}
	int c = kbbuf[kbrd++];
	pthread_mutex_unlock(&conio_mutex);
	return c;
}

void uninit_conio(void)
{
	system("stty echo");
	system("stty icanon");
}

void init_conio(void)
{
	system("stty -echo");
	system("stty -icanon");
	pthread_create(&conio_pid, NULL, conio_routine, NULL);
	atexit(uninit_conio);
}
#endif

void clear(void)
{
#ifdef _WIN32
	system("cls");
#else
	printf("\033[2J");
#endif
}

struct node {
	int x, y;
	struct node *next;
} *head, *tail;
char *map;
int score = 0, X = 18, Y = 18, dx, dy, interval = 800;
#define M(x, y) map[(x) + (X+2)*(y)]

void mkfood(void)
{
	int x, y;
	do {
		x = 1 + rand() % X;
		y = 1 + rand() % Y;
	} while (M(x, y) != ' ');
	M(x, y) = '*';
}

void init(void)
{
	int x, y, size;
	srand((unsigned long)time(NULL));
	size = (X+2) * (Y+2);
	map = malloc(size);
	memset(map, ' ', size);
	for (x = 0; x < X+2; x++)
		M(x, 0) = M(x, Y+1) = '%';
	for (y = 0; y < Y+2; y++)
		M(0, y) = M(X+1, y) = '%';
	do {
		x = 1 + rand() % X;
		y = 1 + rand() % Y;
	} while (M(x, y) != ' ');
	M(x, y) = '@';
	head = malloc(sizeof(struct node));
	head->x = x; head->y = y;
	head->next = head;
	mkfood();
}

void show(void)
{
	int x, y;
	clear();
	puts(message);
	for (y = 0; y < Y+2; y++) {
		for (x = 0; x < X+2; x++) {
			printf("%c ", M(x, y));
		}
		putchar('\n');
	}
	printf("score: %d\n", score);
}

void over(void)
{
	printf("Game Over! Press any key to exit.\n");
	if (kbhit()) getch();
	getch();
	exit(0);
}

void move(void)
{
	int x, y;
	struct node *tail = head->next;
	x = head->x + dx;
	y = head->y + dy;
	switch (M(x, y)) {
		case '@': case ' ':
			M(tail->x, tail->y) = ' ';
			break;
		case '*':
			mkfood();
			tail = malloc(sizeof(struct node));
			tail->next = head->next;
			head->next = tail;
			break;
		default: over();
	}
	tail->x = x; tail->y = y;
	M(head->x, head->y) = '#';
	M(x, y) = '@';
	head = tail;
}

void input(void)
{
	while (kbhit()) switch (getch()) {
	case 'h': case 'a': case 75:
		if (dx != +1)
			dx = -1, dy = 0;
		break;
	case 'j': case 's': case 80:
		if (dy != -1)
			dx = 0, dy = +1;
		break;
	case 'k': case 'w': case 72:
		if (dy != +1)
			dx = 0, dy = -1;
		break;
	case 'l': case 'd': case 77:
		if (dx != -1)
			dx = +1, dy = 0;
		break;
	case 'o':
		interval += 200;
		break;
	case 'i':
		if (interval > 200)
			interval -= 200;
		break;
	case 'p': case ' ':
		printf("Paused. Press any key when you're ready.\n");
		getch();
		break;
	case 'q': case 27:
		printf("quit? [y/N]\n");
		if (getch() == 'y')
			exit(0);
	}
}

int main(int argc, char **argv)
{
	int c;
	if (argc == 3) {
		X = atoi(argv[1]);
		Y = atoi(argv[2]);
	} else if (argc == 2 && !strcmp(argv[1], "-s")) {
		printf("X = ");
		scanf("%d", &X);
		printf("Y = ");
		scanf("%d", &Y);
	}
#ifndef _WIN32
	init_conio();
#endif
	init();
	while (1) {
		show();
		_sleep(interval);
		input();
		move();
	}
}
