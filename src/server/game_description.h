#ifndef H_SERVER_GAME_DESCRIPTION_GUARD
#define H_SERVER_GAME_DESCRIPTION_GUARD

#define PLAYER_WHITE 0
#define PLAYER_BLACK 1

#define GAME_STATE_WAITING_FOR_PLAYERS 0

#define GAME_NAME_MAXSIZE 64

#include <stdint.h>

#include "login_entry.h"
#include "../shared/common.h"

typedef struct game_description{
	uint8_t state;
	uint32_t id;
	char* name;
	login_entry* white;
	login_entry* black;
	dynamic_array* spectators;
	desk_t desk;
} game_description;

int last_game_id;

game_description* init_game_description(int id);
void game_description_set_player(game_description* g, login_entry* l, int color);
int game_description_find(uint32_t id, game_description** g);
void place_fig(desk_t *desk, int type, int team, int i, int j);
void init_desk(game_description *g);

#endif
