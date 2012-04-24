#include <stdlib.h>
#include <stdio.h>

#include "login_entry.h"
#include "lobby.h"
#include "../shared/networking.h"

login_entry* init_login_entry(int id){
	login_entry* log_e;
	if( (log_e = malloc(sizeof(login_entry))) == NULL){
		perror("malloc");
		exit(1);
	}
	if( (log_e->login = malloc(PLAYER_NAME_MAXSIZE)) == NULL){
		perror("malloc");
		exit(1);
	}
	if( (log_e->passw = malloc(ENCRYPTED_PASSWORD_LENGTH)) == NULL){
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
			log_e = (login_entry*)current_lobby.logins->data[i];
			break;
		}
	}
	return log_e;
}

void read_passwords(){
	FILE* file;
	int id;
	login_entry *login;
	if( (file = fopen("passwd", "r")) == NULL){
		perror("fopen");
		exit(1);
	}
	while(!feof(file)){
		fscanf(file, "%d", &id);
		login = init_login_entry(id);
		fscanf(file, "%s", login->login);
		fscanf(file, "%s", login->passw);
		dynamic_array_add(current_lobby.logins, login);
	}
}

void print_passwords(){
	int i;
	login_entry* login;
	for(i = 0; i < current_lobby.logins->size; i++){
		login = (login_entry*)current_lobby.logins->data[i];
		printf("id: %d, login: %s, passw: %s\n", login->id, login->login, login->passw);
		fflush(stdout);
	}
}

