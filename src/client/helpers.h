#ifndef H_CLIENT_HELPERS_GUARD
#define H_CLIENT_HELPERS_GUARD

#include "../shared/common.h"

extern unsigned int ommit_next_autoprompt;

typedef struct tokenized_string_t {
	char *str;
	size_t len;
} tokenized_string_t;

void print_prompt();
int get_string(char **buffer, size_t *length);
int check_command(char *str, const char *command);
int output(const wchar_t *template, ...);

tokenized_string_t tokenize_string(char *str);
char * tokget(tokenized_string_t str, int arg);

void print_desk(desk_t desk);

#endif
