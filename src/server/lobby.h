#ifndef H_SERVER_LOBBY_GUARD
#define H_SERVER_LOBBY_GUARD

#include "dynamic_array.h"

typedef struct lobby{
	dynamic_array* games;
	dynamic_array* logins;
	dynamic_array* sessions;
	dynamic_array* quickstart_players;
} lobby;

lobby current_lobby;

void create_lobby();

#endif
