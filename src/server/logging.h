#ifndef H_SERVER_LOGGING_GUARD
#define H_SERVER_LOGGING_GUARD

#include <time.h>
#include <stdarg.h>

void print_log(const char* thread_info, const char* msg, ...);

#endif
