#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lobby.h"
#include "login_entry.h"
#include "game_description.h"

void read_logins_dump(FILE *logins){
	uint32_t count, login_length, passw_length, id;
	int i;
	login_entry *new_login;
	fread(&count, sizeof(count), 1, logins);
	fread(&last_login_id, sizeof(last_login_id), 1, logins);
	for(i = 0; i < count; i++){
		fread(&id, sizeof(id), 1, logins);
		new_login = init_login_entry(id);
		fread(&login_length, sizeof(login_length), 1, logins);
		fread(new_login->login, login_length, 1, logins);
		new_login->login[login_length] = 0;
		fread(&passw_length, sizeof(passw_length), 1, logins);
		fread(new_login->passw, passw_length, 1, logins);
		dynamic_array_add(current_lobby.logins, new_login);
	}
}

void read_games_dump(FILE *games){
	uint32_t count, spect_size, white_id, black_id;
	uint 8_t white_ex, black_ex;
	int i, j, name_size, id;
	game_description *g;
	fread(&count, sizeof(count), 1, games);
	fread(&last_game_id, sizeof(last_game_id), 1, games);
	for(i = 0; i < count; i++){
		fread(&id, sizeof(id), 1, games);
		g = init_game_description(id);
		fread(&name_size, sizeof(name_size), 1, games);
		fread(g->name, name_size, 1, games);
		g->name[name_size] = 0;
		fread(&g->state, sizeof(g->state), 1, games);
		fread(&white_ex, sizeof(white_ex), 1, games);
		fread(&black_ex, sizeof(black_ex), 1, games);
		if(white_ex){
			fread(&white_id, sizeof(white_id), 1, games);
			g->white = login_entry_find(current_lobby.logins, 
		}
		if(g->black == NULL) black_ex = 0;
		else black_ex = 1;
		fwrite(&white_ex, sizeof(white_ex), 1, games);
		fwrite(&black_ex, sizeof(black_ex), 1, games);
		if(white_ex) fwrite(&g->white->id, sizeof(g->white->id), 1, games);
		if(black_ex) fwrite(&g->black->id, sizeof(g->black->id), 1, games);
		spect_size =  g->spectators->size;
		for(j = 0; j < spect_size; j++)
			fwrite(&((login_entry*)(g->spectators->data[j])->id,
			  sizeof((login_entry*)(g->spectators->data[j])->id), 1, games);
	}
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
	fwrite(&count, sizeof(count), 1, logins);
	fwrite(&last_login_id, sizeof(last_login_id), 1, logins);
	for(i = 0; i < count; i++){
		login = (login_entry*)(current_lobby.logins->data)[i];
		fwrite(&login->id, sizeof(login->id), 1, logins);
		login_length = strlen(login->login);
		fwrite(&login_length, sizeof(login_length), 1, logins);
		fwrite(login->login, login_length, 1, logins);
		passw_length = strlen(login->passw);
		fwrite(&passw_length, sizeof(passw_length), 1, logins);
		fwrite(login->passw, passw_length, 1, logins);
	}
}

void create_games_dump(FILE *games){
	uint32_t count, spect_size;
	uint 8_t white_ex, black_ex;
	int i, j, name_length;
	game_description *g;
	count = current_lobby.games->size;
	fwrite(&count, sizeof(count), 1, games);
	fwrite(&last_game_id, sizeof(last_game_id), 1, games);
	for(i = 0; i < count; i++){
		g = (game_description*)(current_lobby.games->data)[i];
		fwrite(&g->id, sizeof(g->id), 1, games);
		name_length = strlen(g->name);
		fwrite(&name_length, sizeof(name_length), 1, games);
		fwrite(g->name, name_length, 1, games);
		fwrite(&g->state, sizeof(g->state), 1, games);
		if(g->white == NULL) white_ex = 0;
		else white_ex = 1;
		if(g->black == NULL) black_ex = 0;
		else black_ex = 1;
		fwrite(&white_ex, sizeof(white_ex), 1, games);
		fwrite(&black_ex, sizeof(black_ex), 1, games);
		if(white_ex) fwrite(&g->white->id, sizeof(g->white->id), 1, games);
		if(black_ex) fwrite(&g->black->id, sizeof(g->black->id), 1, games);
		spect_size =  g->spectators->size;
		for(j = 0; j < spect_size; j++)
			fwrite(&((login_entry*)(g->spectators->data[j])->id,
			  sizeof((login_entry*)(g->spectators->data[j])->id), 1, games);
	}
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
