#include <stdlib.h>

#include "lobby.h"
#include "game_description.h"
#include "login_entry.h"
#include "session.h"


void create_lobby(){
	if( (passwd = fopen("passwd", "a+")) == NULL){
		perror("fopen");
		exit(1);
	}
	current_lobby.games = init_dynamic_array(sizeof(game_description));
	current_lobby.logins = init_dynamic_array(sizeof(login_entry));
	current_lobby.sessions = init_dynamic_array(sizeof(session));
	current_lobby.quickstart_players = init_dynamic_array(sizeof(int));
}

