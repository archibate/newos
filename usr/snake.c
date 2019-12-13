#if 0 ////////////////////////////////////////// {{{
true /*; chmod +x $0
if grep -q 'math\.h' $0; then C+=-lm; fi
if grep -q 'pthread\.h' $0; then C+=-lpthread; fi
if [ ! -z $GDB ]; then C+="-gstabs+ -ggdb -D_DEBUG"; fi
gcc $C -Werror $0 -o /tmp/$$ && $GDB /tmp/$$ $*; x=$?
rm -f /tmp/$$; exit $x
true CCSH_signature123 */
#endif
// # }}} [3J[H[2J
#ifdef _NEWOS
#include "busybox.h"
#endif
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
#include <windows.h>
#define _sleep(x) Sleep(x)
#else
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#ifdef _NEWOS
#include <sys/notify.h>
#define _sleep(x) sleep(((x)+999)/1000)
#else
#include <pthread.h>
#define _sleep(x) usleep((x)*1000)
#endif

#ifndef _NEWOS
char kbbuf[256];
unsigned char kbwr, kbrd;
pthread_t conio_pid;
pthread_mutex_t conio_mutex = PTHREAD_MUTEX_INITIALIZER;

static int kbhit(void)
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

static int getch(void)
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
#else
static int kbready;

static void sigpoll(int sig)
{
	signal(SIGPOLL, sigpoll);
	ionotify(0, ION_READ, 0);
	kbready = 1;
}

static int kbhit(void)
{
	return kbready;
}

static int getch(void)
{
	int c = getchar();
	kbready = 0;
	return c;
}
#endif

static struct termios tc_orig, tc_raw;

static void uninit_conio(void)
{
	tcsetattr(0, TCSANOW, &tc_orig);
}

static void init_conio(void)
{
	if (tcgetattr(0, &tc_orig) == -1) {
		fprintf(stderr, "error: input not from a terminal\n");
		exit(1);
	}
	memcpy(&tc_raw, &tc_orig, sizeof(struct termios));
	tc_raw.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &tc_raw);
	atexit(uninit_conio);
#ifndef _NEWOS
	pthread_create(&conio_pid, NULL, conio_routine, NULL);
#else
	signal(SIGPOLL, sigpoll);
	if (ionotify(0, ION_READ, 0) == -1) {
		perror("cannot ionotify stdin");
		exit(1);
	}
#endif
	signal(SIGINT, exit);
}
#endif

static void clear(void)
{
#ifdef _WIN32
	COORD c = {0, 0};
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsole, c);
#else
#ifdef _NEWOS
	printf("\033[H");
#else
	printf("\033[2J");
#endif
#endif
}

static struct node {
	int x, y;
	struct node *next;
} *head, *tail;
static char *map;
static int score = 0, X = 18, Y = 18, dx, dy, interval = 800;
#define M(x, y) map[(x) + (X+2)*(y)]

static void mkfood(void)
{
	int x, y;
	do {
		x = 1 + rand() % X;
		y = 1 + rand() % Y;
	} while (M(x, y) != ' ');
	M(x, y) = '*';
}

static void init(void)
{
	int x, y, size;
#ifndef _NEWOS
	srand((unsigned long)time(NULL));
#endif
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

static void show(void)
{
	int x, y;
	clear();
#ifndef _WIN32
#ifndef _NEWOS
	puts(message);
#endif
#endif
	for (y = 0; y < Y+2; y++) {
		for (x = 0; x < X+2; x++) {
			printf("%c ", M(x, y));
		}
		putchar('\n');
	}
	printf("score: %d\n", score);
}

static void over(void)
{
#ifdef _WIN32
	printf("Game Over! Press any key to exit.\n");
	while (kbhit()) getch();
	getch();
#else
	printf("Game Over!\n");
#endif
	exit(0);
}

static void move(void)
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
			score += 1;
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

static void input(void)
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
#ifndef _NEWOS
	} else if (argc == 2 && !strcmp(argv[1], "-s")) {
		printf("X = ");
		scanf("%d", &X);
		printf("Y = ");
		scanf("%d", &Y);
#endif
	}
#ifndef _WIN32
	init_conio();
#endif
	init();
#ifdef _NEWOS
	printf("\033[2J");
#endif
	while (1) {
		show();
		_sleep(interval);
		input();
		move();
	}
}
