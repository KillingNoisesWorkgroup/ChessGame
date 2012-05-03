#ifndef H_CLIENT_HELPERS_GUARD
#define H_CLIENT_HELPERS_GUARD

void print_prompt();
int get_string(char **buffer, size_t *length);
int check_command(char *str, const char *command);
int output(const char *template, ...);

#endif
