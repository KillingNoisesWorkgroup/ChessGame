#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "helpers.h"

void print_prompt() {
	// print user prompt
	printf("[%s]$ ", session.login);
}

int get_string(char **buffer, size_t *length) {
	int read;
	
	*buffer = NULL;
	if((read = getline(buffer, length, stdin)) == -1) {
		return 0;		
	}
	
	// We must guarantee null-terminated string
	if((*buffer)[*length - 1] != '\0') {
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
