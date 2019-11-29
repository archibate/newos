#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <bits/types.h>

typedef __id_t pthread_t;
typedef struct pthread_attr {
};

int pthread_create(pthread_t *tid, const pthread_attr_t *attr,
		void (*start)(void *), void *arg);
int pthread_detach(pthread_t tid);
pthread_t pthread_self(void);

#endif
