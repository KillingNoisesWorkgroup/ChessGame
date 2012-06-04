#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lobby.h"
#include "login_entry.h"
#include "game_description.h"
#include "game_log.h"

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
	uint32_t count, white_id, black_id;
	int i, j, name_size, id, spect_id;
	game_description *g;
	login_entry *spectator;
	fread(&count, sizeof(count), 1, games);
	fread(&last_game_id, sizeof(last_game_id), 1, games);
	for(i = 0; i < count; i++){
		fread(&id, sizeof(id), 1, games);
		g = init_game_description(id);
		fread(&name_size, sizeof(name_size), 1, games);
		fread(g->name, name_size, 1, games);
		g->name[name_size] = 0;
		g->game_log = open_game_log(g->id);
		fread(&g->state, sizeof(g->state), 1, games);
		fread(&g->moves_made, sizeof(g->moves_made), 1, games);
		fread(&white_id, sizeof(white_id), 1, games);
		fread(&black_id, sizeof(black_id), 1, games);
		if(white_id){
			if(login_entry_find_id(white_id, &g->white) == -1) g->white = NULL;
		}
		if(black_id){
			if(login_entry_find_id(black_id, &g->black) == -1) g->black = NULL;
		}
		fread(&g->spectators->size, sizeof(g->spectators->size), 1, games);
		for(j = 0; j < g->spectators->size; j++){
			fread(&spect_id, sizeof(spect_id), 1, games);
			login_entry_find_id(spect_id, &spectator);
			dynamic_array_add(g->spectators, spectator);
		}
		for(j = 0; j < 64; j++){
			fread(&(g->desk.cells[j].type), sizeof(g->desk.cells[j].type), 1, games);
			fread(&(g->desk.cells[j].color), sizeof(g->desk.cells[j].color), 1, games);
		}
		dynamic_array_add(current_lobby.games, g);
	}
}

void read_memory_dump(){
	FILE *logins, *games;
	if( (logins = fopen("dumps/logins", "r")) != NULL){
		read_logins_dump(logins);
		fclose(logins);
	}
	if( (games = fopen("dumps/games", "r")) != NULL){
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
		passw_length = ENCRYPTED_PASSWORD_LENGTH * 2;
		fwrite(&passw_length, sizeof(passw_length), 1, logins);
		fwrite(login->passw, passw_length, 1, logins);
	}
}

void create_games_dump(FILE *games){
	uint32_t count, spect_size, white_id, black_id;
	int i, j, name_length;
	game_description *g;
	count = current_lobby.games->size;
	fwrite(&count, sizeof(count), 1, games);
	fwrite(&last_game_id, sizeof(last_game_id), 1, games);
	for(i = 0; i < count; i++){
		g = (game_description*)current_lobby.games->data[i];
		fwrite(&g->id, sizeof(g->id), 1, games);
		name_length = strlen(g->name);
		fwrite(&name_length, sizeof(name_length), 1, games);
		fwrite(g->name, name_length, 1, games);
		fwrite(&g->state, sizeof(g->state), 1, games);
		fwrite(&g->moves_made, sizeof(g->moves_made), 1, games);
		if(g->white == NULL) white_id = 0;
		else white_id = g->white->id;
		if(g->black == NULL) black_id = 0;
		else black_id = g->black->id;
		fwrite(&white_id, sizeof(white_id), 1, games);
		fwrite(&black_id, sizeof(black_id), 1, games);
		spect_size =  g->spectators->size;
		fwrite(&spect_size, sizeof(spect_size), 1, games);
		for(j = 0; j < spect_size; j++)
			fwrite(&((login_entry*)(g->spectators->data[j]))->id,
			  sizeof(((login_entry*)(g->spectators->data[j]))->id), 1, games);
		for(j = 0; j < 64; j++){
			fwrite(&(g->desk.cells[j].type), sizeof(g->desk.cells[j].type), 1, games);
			fwrite(&(g->desk.cells[j].color), sizeof(g->desk.cells[j].color), 1, games);
		}
	}
}

void create_memory_dump(){
	FILE *logins, *games;
	mkdir("dumps");
	if( (logins = fopen("dumps/logins", "w+")) == NULL){
		perror("fopen");
		exit(1);
	}
	if( (games = fopen("dumps/games", "w+")) == NULL){
		perror("fopen");
		exit(1);
	}
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	create_logins_dump(logins);
	create_games_dump(games);
	
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	
	fclose(logins);
	fclose(games);
}

void* DumpsThread(void* arg){
	int dumps_creation_frequency;
	dumps_creation_frequency = (int)arg;
	while(1){
		sleep(dumps_creation_frequency);
		print_log("dump creation thread", "Started dump creation");
		create_memory_dump();
		print_log("dump creation thread", "Finished dump creation");
	}
}

void create_dump_thread(int dump_creation_frequency){
	int tmp;
	pthread_t thread;
	tmp = dump_creation_frequency;
	if( (pthread_create(&thread, NULL, (void*)DumpsThread, (void*)tmp)) != 0){
		perror("pthread_create");
		exit(1);
	}
}
