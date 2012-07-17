#ifndef _LOG_H
#define _LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

#define FREE(p) do{if(p) free(p);p = NULL;}while(0)

typedef enum log_level {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR
} log_level;

void test_log(log_level level, int errnum, FILE *fd, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
