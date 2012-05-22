#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#include "client.h"
#include "helpers.h"
#include "../shared/helpers.h"

unsigned int ommit_next_autoprompt = 0;

void print_prompt() {
	// print user prompt
	if(session.state.current == GAMESTATE_INGAME && session.state.game_name) {
		output(L"[%s][%s]$ ", session.login, session.state.game_name);
	} else {
		output(L"[%s]$ ", session.login);
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

int output(const wchar_t *template, ...) {
	va_list ap;
	int val;
	
	va_start(ap, template);
    val = vfwprintf(stdout, template, ap);
    va_end(ap);
	
	fflush(stdout);
	return val;
}

// Internal helper
void print_figure(uint8_t type, uint8_t color) {
	#ifdef LETTERS
	switch(color) {
		case FIGURE_COLOR_WHITE:
			wprintf(L"\033[37;1m");
			break;
		case FIGURE_COLOR_BLACK:
			wprintf(L"\033[30;1m");					
			break;
	}
	
	switch(type) {
	  case FIGURE_KING:
		wprintf(L"K");
		break;
	  case FIGURE_QUEEN:
		wprintf(L"Q");
		break;
	  case FIGURE_ROOK:
		wprintf(L"R");
		break;
	  case FIGURE_KNIGHT:
		wprintf(L"N");
		break;
	  case FIGURE_BISHOP:
		wprintf(L"B");
		break;
	  case FIGURE_PAWN:
		wprintf(L"P");
		break;
	  default:
		wprintf(L" ");
		break;		
	}
	
	wprintf(L"\033[0m");
	#else
	switch(color) {
		case FIGURE_COLOR_WHITE:
			wprintf(L"\033[37;1m");
			break;
		case FIGURE_COLOR_BLACK:
			wprintf(L"\033[30;1m");					
			break;
	}
	
	switch(type) {
	  case FIGURE_KING:
		wprintf(L"%lc", 0x265B);
		break;
	  case FIGURE_QUEEN:
		wprintf(L"%lc", 0x265A);
		break;
	  case FIGURE_ROOK:
		wprintf(L"%lc", 0x265C);
		break;
	  case FIGURE_KNIGHT:
		wprintf(L"%lc", 0x265D);
		break;
	  case FIGURE_BISHOP:
		wprintf(L"%lc", 0x265E);
		break;
	  case FIGURE_PAWN:
		wprintf(L"%lc", 0x265F);
		break;
	  default:
		wprintf(L" ");
		break;		
	}
	
	wprintf(L"\033[0m");	
	#endif
	fflush(stdout);
}

void print_desk(desk_t desk) {	
	int i, j;
	int cycle = 1;
	
	wprintf(L"\n");
	
	for(i = 0; i < 8; i++) {
		cycle = !cycle;
		//printf("%d |", (7 - i) + 1);
		wprintf(L" %d ", (7 - i) + 1);
		
		for(j = 0; j < 8; j++) {
			cycle = !cycle;
			cycle ? wprintf(L"\033[47m") : wprintf(L"\033[40m");
			wprintf(L" ");
			
			cell_t * cell = &desk.cells[(7 - i)*8 + j];
			print_figure(cell->type, cell->color);
			
			wprintf(L"\033[0m");
		}
		
		wprintf(L"\n");
	}
	
	wprintf(L"   ");
	for(i = 0; i < 8; i++) wprintf(L" %c", num_to_char(i + 1));
	
	wprintf(L"\n");
	
	print_prompt();
}
