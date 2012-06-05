#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

void print_log(const char* thread_info, const char* msg, ...){
	time_t rawtime;
	struct tm * timeinfo;
	va_list va;
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	va_start(va, msg);
	
	printf("%s [%02d:%02d:%02d]: ", thread_info, timeinfo->tm_hour,
		timeinfo->tm_min, timeinfo->tm_sec);
	vprintf(msg, va);
	printf("\n");
	
	va_end(va);
}
