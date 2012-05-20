#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "client.h"
#include "helpers.h"

unsigned int ommit_next_autoprompt = 0;

void print_prompt() {
	// print user prompt
	if(session.state.current == GAMESTATE_INGAME && session.state.game_name) {
		output("[%s][%s]$ ", session.login, session.state.game_name);
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

// Internal helper
char figure_to_char(uint8_t figure_type) {
	switch(figure_type) {
	  case FIGURE_KING:
		return 'K';
	  case FIGURE_QUEEN:
		return 'Q';
	  case FIGURE_ROOK:
		return 'Q';
	  case FIGURE_KNIGHT:
		return 'K';
	  case FIGURE_BISHOP:
		return 'Q';
	  case FIGURE_PAWN:
		return 'P';
	}
	return ' ';
}

void print_desk(desk_t desk) {	
	int i, j;
	int cycle = 0;
	
	for(i = 0; i < 8; i++) {
		cycle = !cycle;
		printf("%d |", (7 - i) + 1);
		
		for(j = 0; j < 8; j++) {
			cycle = !cycle;
			cycle ? printf("\033[43m") : printf("\033[40m");
			printf(" ");
			
			cell_t * cell = &desk.cells[(7 - i)*8 + j];
			
			switch(cell->color) {
				case FIGURE_COLOR_WHITE:
					printf("\033[32;1m");
					break;
				case FIGURE_COLOR_BLACK:
					printf("\033[37;1m");					
					break;
			}
			
			printf("%c", figure_to_char(cell->type));
			
			printf("\033[0m");
		}
		
		printf("\n");
	}
	
	printf("--|");
	
	for(i = 0; i < 8; i++) printf("--");
	printf("\n");
	printf("  |");
	for(i = 0; i < 8; i++) printf(" %c", num_to_char(i + 1));
	
	printf("\n");
	
	print_prompt();
}
