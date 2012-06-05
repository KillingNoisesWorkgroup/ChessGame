#ifndef H_CLIENT_GAMESTATE_GUARD
#define H_CLIENT_GAMESTATE_GUARD

#include "../shared/networking.h"

typedef enum gamestates {
	GAMESTATE_NONE,
	GAMESTATE_LOBBY,
	GAMESTATE_INGAME
} gamestates; 

typedef struct gamestate_rec {
	gamestates current;
	char game_name[MAX_PACKET_STRING_LENGTH];
	uint8_t team;
	uint8_t desk_inverted;
} gamestate_rec;

#endif