#ifndef H_SERVER_LOGIN_ENTRY_GUARD
#define H_SERVER_LOGIN_ENTRY_GUARD

#include <stdio.h>

#include "dynamic_array.h"

#define ADMIN_ID 0

typedef struct login_entry{
	char *login;
	char *passw;
	int id;
} login_entry;

FILE* passwd;

login_entry* init_login_entry(int id);
int login_entry_find(char* login, login_entry** l);
void read_passwords();
void print_passwords();
void login_entry_register(int id, char *login, char * passw);
int isadmin(login_entry* l);

#endif
