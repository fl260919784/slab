#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdio.h>

#define likely(x)                   __builtin_expect(!!(x), 1)
#define unlikely(x)                 __builtin_expect(!!(x), 0)


#if DEBUG
	#define log_debug(format, ...) printf (format, ##__VA_ARGS__)
	#define log_warning(format, ...) printf (format, ##__VA_ARGS__)
	#define log_error(format, ...) printf (format, ##__VA_ARGS__)
#endif


#if WARNING
	#undef log_warning
	#undef log_error

	#define log_warning(format, ...) printf (format, ##__VA_ARGS__)
	#define log_error(format, ...) printf (format, ##__VA_ARGS__)
#endif

#if ERROR
	#undef log_error
	#define log_error(format, ...) printf (format, ##__VA_ARGS__)
#endif


#ifndef log_debug
	#define log_debug(format,...)
#endif
#ifndef log_warning
#define log_warning(format, ...)
#endif
#ifndef log_error
#define log_error(format,...)
#endif

#endif