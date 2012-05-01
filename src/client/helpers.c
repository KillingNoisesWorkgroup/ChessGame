#include <stdio.h>

#include "client.h"
#include "helpers.h"

void print_prompt() {
	// print user prompt
	printf("[%s]$ ", session.login);
}

int get_command(char **buffer, int *length) {
	int read;
	
	*buffer = NULL;
	if((read = getline(buffer, length, stdin)) == -1) {
		return 0;		
	}
	return 1;
}