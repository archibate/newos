#ifndef _GETOPT_H
#define _GETOPT_H 1

int getopt(int argc, char *const *argv, const char *optstr);
extern char *optarg;
extern int optind, opterr, optopt;

#endif
