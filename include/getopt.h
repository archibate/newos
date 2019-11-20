#ifndef _GETOPT_H
#define _GETOPT_H 1

#ifndef _LIBC_EXP
#define optarg (*IMP$optarg)
#define optind (*IMP$optind)
#define opterr (*IMP$opterr)
#define optopt (*IMP$optopt)
#endif
int getopt(int argc, char *const *argv, const char *optstr);
extern char *optarg;
extern int optind, opterr, optopt;

#endif
