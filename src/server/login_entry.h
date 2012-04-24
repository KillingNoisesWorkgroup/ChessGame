#ifndef H_SERVER_LOGIN_ENTRY_GUARD
#define H_SERVER_LOGIN_ENTRY_GUARD

#include "dynamic_array.h"

typedef struct login_entry{
	char *login;
	char *passw;
	int id;
} login_entry;

login_entry* init_login_entry(int id);
login_entry* login_entry_find(char* login);

#endif

