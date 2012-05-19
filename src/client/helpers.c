#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "client.h"
#include "helpers.h"

void print_prompt() {
	// print user prompt
	if(session.state.current == GAMESTATE_INGAME && session.state.current_game_name) {
		output("[%s][%s]$ ", session.login, session.state.current_game_name);
	} else {
		output("[%s]$ ", session.login);
	}
}

int get_string(char **buffer, size_t *length) {
	size_t read;
	
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

tokenized_string_t tokenize_string(char *str) {
	tokenized_string_t value;
	int i;
	
	value.len = strlen(str);
	if((value.str = malloc(value.len + 1)) == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
			
	for(i = 0; i < value.len; i++) {
		if(str[i] == ' ' || str[i] == '\n' || str[i] == '\t')
			value.str[i] = '\0';
		else
			value.str[i] = str[i];
	}
	value.str[value.len] = '\0';
	
	return value; 
}

char * tokget(tokenized_string_t str, int arg) {
	int i, n;
	
	n = 0;
	for(i = 0; i < str.len; i++) {
		if(n >= arg)
			return str.str + i;
		if(str.str[i] == '\0')
			n++;
	}
	
	return NULL; 
}

int output(const char *template, ...) {
	va_list ap;
	int val;
	
	va_start(ap, template);
    val = vfprintf(stdout, template, ap);
    va_end(ap);
	
	fflush(stdout);
	return val;
}
