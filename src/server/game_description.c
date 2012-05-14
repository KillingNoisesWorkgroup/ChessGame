#include <stdlib.h>
#include <stdio.h>

#include "game_description.h"
#include "lobby.h"
#include "dynamic_array.h"
#include "login_entry.h"

game_description* init_game_description(int id){
	game_description* g;
	if((g = malloc(sizeof(game_description))) == NULL){
		perror("malloc");
		exit(1);
	}
	if((g->name = malloc(GAME_NAME_MAXSIZE + 1)) == NULL){
		perror("malloc");
		exit(1);
	}
	g->white = NULL;
	g->black = NULL;
	g->spectators = init_dynamic_array(sizeof(login_entry));
	g->id = (uint32_t)id;
	g->state = GAME_STATE_WAITING_FOR_PLAYERS;
	return g;
}

void game_description_set_player(game_description* g, login_entry* l, int color){
	if(color == PLAYER_WHITE) g->white = l;
	if(color == PLAYER_BLACK) g->black = l;
}

int game_description_find(uint32_t id, game_description** g){
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
