#include "log.h"

#ifdef USE_LOGCOLOR
#define NORMAL   "\033[0m"
#define COLOR(x) "\033[" #x ";1m"
#define BOLD     COLOR(1)
#define F_RED    COLOR(31)
#define F_GREEN  COLOR(32)
#define F_YELLOW COLOR(33)
#define F_BLUE   COLOR(34)
#define B_RED    COLOR(41)
#endif /* USE_LOGCOLOR */

void test_log(log_level level, int errnum, FILE *fd, const char *format, ...)
{
	if (format == NULL || fd == NULL)
		return;

#ifdef USE_LOGCOLOR
	static const char *const c[] =
	{
		[MSG_INFO]    = F_GREEN,
		[MSG_WARNING] = F_YELLOW,
		[MSG_ERROR]   = F_RED
	};
#endif 
	static const char *const l[] =
	{
		[MSG_INFO]    = "Info",
		[MSG_WARNING] = "Warn",
		[MSG_ERROR]   = "Err"
	};

	char *error = NULL;
	if (errnum != 0)
	{
		error = strerror(errnum);
		errno = 0;
	}

	char fmt[256] = {0};
	if (error != NULL)
	{
#ifdef USE_LOGCOLOR
		snprintf(fmt, sizeof(fmt), "[test] %s%s: %s %s\n", 
				c[level], l[level], format, error);
#else
		snprintf(fmt, sizeof(fmt), "[test] %s: %s %s\n", 
				l[level], format, error);
#endif
	}
	else
	{
#ifdef USE_LOGCOLOR
		snprintf(fmt, sizeof(fmt), "[test] %s%s: %s\n", 
				c[level], l[level], format);
#else
		snprintf(fmt, sizeof(fmt), "[test] %s: %s\n", 
				l[level], format);
#endif
	}

	va_list va;
	va_start(va, format);
	vfprintf(fd, fmt, va);
	va_end(va);
}

