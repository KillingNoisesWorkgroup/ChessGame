#include <stdlib.h>
#include <stdio.h>

#include "game_description.h"
#include "lobby.h"
#include "dynamic_array.h"
#include "login_entry.h"
#include "game_log.h"

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
	g->moves_made = 0;
	g->game_log = open_game_log(g->id);
	g->state = GAME_STATE_WAITING_FOR_PLAYERS;
	init_desk(g);
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

void place_fig(desk_t *desk, uint8_t type, uint8_t color, uint8_t i, uint8_t j){
	desk->cells[i + j*8].type = type;
	desk->cells[i + j*8].color = color;
}

void move_fig(desk_t *desk, uint8_t from_letter, uint8_t from_number, uint8_t to_letter, uint8_t to_number){
	cell_t fig;
	fig = desk->cells[from_letter + from_number*8];
	place_fig(desk, fig.type, fig.color, to_letter, to_number);
	place_fig(desk, FIGURE_NONE, FIGURE_COLOR_NONE, from_letter, from_number);
}

void init_desk(game_description *g){
	cell_t c;
	int i, j;
	place_fig(&g->desk, FIGURE_ROOK, FIGURE_COLOR_WHITE, 0, 0);
	place_fig(&g->desk, FIGURE_KNIGHT, FIGURE_COLOR_WHITE, 1, 0);
	place_fig(&g->desk, FIGURE_BISHOP, FIGURE_COLOR_WHITE, 2, 0);
	place_fig(&g->desk, FIGURE_QUEEN, FIGURE_COLOR_WHITE, 3, 0);
	place_fig(&g->desk, FIGURE_KING, FIGURE_COLOR_WHITE, 4, 0);
	place_fig(&g->desk, FIGURE_BISHOP, FIGURE_COLOR_WHITE, 5, 0);
	place_fig(&g->desk, FIGURE_KNIGHT, FIGURE_COLOR_WHITE, 6, 0);
	place_fig(&g->desk, FIGURE_ROOK, FIGURE_COLOR_WHITE, 7, 0);
	for(i = 0; i < 8; i++){
		place_fig(&g->desk, FIGURE_PAWN, FIGURE_COLOR_WHITE, i, 1);
	}
	
	place_fig(&g->desk, FIGURE_ROOK, FIGURE_COLOR_BLACK, 0, 7);
	place_fig(&g->desk, FIGURE_KNIGHT, FIGURE_COLOR_BLACK, 1, 7);
	place_fig(&g->desk, FIGURE_BISHOP, FIGURE_COLOR_BLACK, 2, 7);
	place_fig(&g->desk, FIGURE_QUEEN, FIGURE_COLOR_BLACK, 3, 7);
	place_fig(&g->desk, FIGURE_KING, FIGURE_COLOR_BLACK, 4, 7);
	place_fig(&g->desk, FIGURE_BISHOP, FIGURE_COLOR_BLACK, 5, 7);
	place_fig(&g->desk, FIGURE_KNIGHT, FIGURE_COLOR_BLACK, 6, 7);
	place_fig(&g->desk, FIGURE_ROOK, FIGURE_COLOR_BLACK, 7, 7);
	for(i = 0; i < 8; i++){
		place_fig(&g->desk, FIGURE_PAWN, FIGURE_COLOR_BLACK, i, 6);
	}
	
	for(j = 2; j < 6; j++){
		for(i = 0; i < 8; i++){
			place_fig(&g->desk, FIGURE_NONE, FIGURE_COLOR_NONE, i, j);
		}
	}
}
