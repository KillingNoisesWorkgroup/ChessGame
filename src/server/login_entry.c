#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "login_entry.h"
#include "lobby.h"
#include "../shared/networking.h"

login_entry* init_login_entry(int id){
	login_entry* log_e;
	if( (log_e = malloc(sizeof(login_entry))) == NULL){
		perror("malloc");
		exit(1);
	}
	if( (log_e->login = malloc(PLAYER_NAME_MAXSIZE + 1)) == NULL){
		perror("malloc");
		exit(1);
	}
	if( (log_e->passw = malloc(ENCRYPTED_PASSWORD_LENGTH * 2 + 1)) == NULL){
		perror("malloc");
		exit(1);
	}
	log_e->id = id;
	return log_e;
}

login_entry* login_entry_find(char* login){
	login_entry* log_e;
	int i;
	log_e = NULL;
	for(i = 0; i < current_lobby.logins->size; i++){
		if( strcmp( ((login_entry*)(current_lobby.logins->data[i]))->login, login) == 0){
			log_e = (login_entry*)(current_lobby.logins->data[i]);
			break;
		}
	}
	return log_e;
}

void print_passwords(){
	int i;
	login_entry *l;
	for(i = 0; i < current_lobby.logins->size; i++){
		l = (login_entry*)(current_lobby.logins->data[i]);
		printf("id %d, login %s, passw %s\n", l->id, l->login, l->passw);
		fflush(stdout);
	}
}
