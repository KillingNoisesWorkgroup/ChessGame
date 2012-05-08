#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lobby.h"
#include "login_entry.h"

void read_logins_dump(FILE *logins){
	uint32_t count, login_length, passw_length;
	int i, id;
	login_entry *new_login;
	fread((void*)&count, sizeof(count), 1, logins);
	for(i = 0, id = 0; i < count; i++){
		new_login = init_login_entry(id++);
		fread((void*)&login_length, sizeof(login_length), 1, logins);
		fread((void*)(new_login->login), login_length, 1, logins);
		new_login->login[login_length] = 0;
		fread((void*)&passw_length, sizeof(passw_length), 1, logins);
		fread((void*)(new_login->passw), passw_length, 1, logins);
		dynamic_array_add(current_lobby.logins, new_login);
	}
}

void read_games_dump(FILE *games){
	
}

void read_memory_dump(){
	FILE *logins, *games;
	if( (logins = fopen("logins.dump", "r")) != NULL){
		read_logins_dump(logins);
		fclose(logins);
	}
	if( (games = fopen("games.dump", "r")) != NULL){
		read_games_dump(games);
		fclose(games);
	}
}

void create_logins_dump(FILE *logins){
	uint32_t count, login_length, passw_length;
	int i;
	login_entry *login;
	count = current_lobby.logins->size;
	fwrite((const void*)&count, sizeof(count), 1, logins);
	for(i = 0; i < count; i++){
		login = (login_entry*)(current_lobby.logins->data)[i];
		login_length = strlen(login->login);
		fwrite((const void*)&login_length, sizeof(login_length), 1, logins);
		fwrite((const void*)login->login, login_length, 1, logins);
		passw_length = strlen(login->passw);
		fwrite((const void*)&passw_length, sizeof(passw_length), 1, logins);
		fwrite((const void*)login->passw, passw_length, 1, logins);
	}
}

void create_games_dump(FILE *games){
	
}

void create_memory_dump(){
	FILE *logins, *games;
	if( (logins = fopen("logins.dump", "w+")) == NULL){
		perror("fopen");
		exit(1);
	}
	if( (games = fopen("games.dump", "w+")) == NULL){
		perror("fopen");
		exit(1);
	}
	create_logins_dump(logins);
	create_games_dump(games);
	fclose(logins);
	fclose(games);
}
