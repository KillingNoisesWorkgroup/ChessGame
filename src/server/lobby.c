#include <stdlib.h>

#include "lobby.h"
#include "game_description.h"
#include "login_entry.h"
#include "session.h"
#include "memory_dump.h"

void create_lobby(){
	current_lobby.games = init_dynamic_array(sizeof(game_description));
	current_lobby.logins = init_dynamic_array(sizeof(login_entry));
	current_lobby.sessions = init_dynamic_array(sizeof(session));
	current_lobby.quickstart_players = init_dynamic_array(sizeof(int));
	read_memory_dump();
}
