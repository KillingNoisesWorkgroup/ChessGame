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
	printf("searching for login %s\n", login);
	printf("logins array size is %d\n", current_lobby.logins->size);
	for(i = 0; i < current_lobby.logins->size; i++){
		if( strcmp( ((login_entry*)(current_lobby.logins->data[i]))->login, login) == 0){
			log_e = (login_entry*)(current_lobby.logins->data[i]);
			printf("login found\n");
			break;
		}
	}
	return log_e;
}

void read_passwords(){
	int id, read, length;
	char* buf = NULL;
	login_entry *login;
	printf("reading passwords\n");
	while((read = getline(&buf, &length, passwd)) != -1){
		fflush(stdout);
		if(read <= 0) break;
		login = init_login_entry(0);
		sscanf(buf, "%d %s %s", &id, login->login, login->passw);
		login->id = id;
		dynamic_array_add(current_lobby.logins, login);
		free(buf);
		buf = NULL;
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

void login_entry_register(int id, char* login, char* passw){
	fprintf(passwd, "\n%d ", id);
	fprintf(passwd, "%s ", login);
	fprintf(passwd, "%s", passw);
}

