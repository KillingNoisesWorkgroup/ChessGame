#include <stdlib.h>
#include <stdio.h>

#include "game_description.h"
#include "lobby.h"

game_description* init_game_description(int id, char *name){
	game_description* g;
	if((g = malloc(sizeof(game_description))) == NULL){
		perror("malloc");
		exit(1);
	}
	g->name = name;
	g->white = NULL;
	g->black = NULL;
	g->id = id;
	g->state = GAME_STATE_WAITING_FOR_PLAYERS;
	return g;
}

void game_description_set_player(game_description* g, login_entry* l, int color){
	if(color == PLAYER_WHITE) g->white = l;
	if(color == PLAYER_BLACK) g->black = l;
}

int game_description_find(int id, game_description** g){
	game_description* game_d;
	int i, b = 0;
	game_d = NULL;
	for(i = 0; i < current_lobby.games->size; i++){
		if(((game_description*)(current_lobby.games->data[i]))->id == id){
			game_d = (game_description*)(current_lobby.games->data[i]);
			b = 1;
			break;
		}
	}
	*g = game_d;
	if(b) return i;
	else return -1;
}
