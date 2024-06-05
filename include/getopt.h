#ifndef _GETOPT_H
#define _GETOPT_H 1

int getopt(int argc, char *const *argv, const char *optstr);

struct _getopt {
    char *_optarg;
    int _optind, _opterr, _optopt;
};

struct _getopt *__getopt_location();

#define optarg (__getopt_location()->_optarg)
#define optind (__getopt_location()->_optind)
#define opterr (__getopt_location()->_opterr)
#define optopt (__getopt_location()->_optopt)

#endif
