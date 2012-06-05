#ifndef H_SERVER_LOGIN_ENTRY_GUARD
#define H_SERVER_LOGIN_ENTRY_GUARD

#include <stdio.h>
#include <stdint.h>

#include "dynamic_array.h"

#define ADMIN_ID 1

typedef struct login_entry{
	char *login;
	char *passw;
	uint32_t id;
	int rating;
} login_entry;

FILE* passwd;
int last_login_id;

login_entry* init_login_entry(uint32_t id);
int login_entry_find(char* login, login_entry** l);
int login_entry_find_id(uint32_t id, login_entry** l);
void read_passwords();
void print_passwords();
void login_entry_register(uint32_t id, char *login, char * passw);
int isadmin(login_entry* l);

#endif
