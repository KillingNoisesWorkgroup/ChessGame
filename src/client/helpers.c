#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "client.h"
#include "helpers.h"

void print_prompt() {
	// print user prompt
	output("[%s]$ ", session.login);
}

int get_string(char **buffer, size_t *length) {
	int read;
	
	*buffer = NULL;
	if((*length = getline(buffer, &read, stdin)) == -1) {
		return 0;		
	}
	
	// We must guarantee null-terminated string
	if((*buffer)[*length] != '\0') {
		perror("getline");
		exit(EXIT_FAILURE);
	}
	
	return 1;
}

int check_command(char *str, const char *command) {
	if(str == NULL || command == NULL)
		return 0;
	
	for(; *command != '\0' && 
			*str != '\0' && 
			*str != ' ' && 
			*str != '\t' && 
			*str != '\n' && 
			*command == *str; str++, command++);
	return *command == '\0';
}

int output(const char *template, ...) {
	va_list ap;
	int val;
	
	va_start(ap, template);
    val = vfprintf(stdout, template, ap);
    va_end(ap);
	return val;
}
