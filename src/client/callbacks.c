#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // for htonl() and etc

#include "client.h"
#include "helpers.h"
#include "callbacks.h"
#include "packet_helpers.h"

// Some helpers to fancy up the code
#define on if(0){}
#define command(tested_command) else if(strcmp(tokget(cmd, 0), (tested_command)) == 0)
#define packet(type) else if(ptype == type)
#define tokenizer_init tokenized_string_t cmd = tokenize_string(buff);
#define tokenizer_free free(cmd.str);

// Variable names must be all the same as in the prototype for 
// fancy helpers to work properly

// Callbacks

void cb_remote_ingame(int ptype, int plen, void *payload) {
	on packet(PACKET_GENERAL_STRING) {
	
		// Force string end
		((char *)payload)[plen - 1] = '\0';
		output("%s\n", (char *)payload);
		print_prompt();
	
	} else {
		output("Unknown packet (ingame)!\n");
		packet_debug(ptype, plen, payload);
		print_prompt();
	}
}

void cb_remote_default(int ptype, int plen, void *payload) {
	on packet(PACKET_GAME_ATTACH) {
	
		packet_game_attach * p = payload;
		int gameid = ntohl(p->gameid);
		
		reactor.callback_remote = &cb_remote_ingame;
		session.state.current = GAMESTATE_INGAME;
		snprintf(session.state.game_name, sizeof session.state.game_name, "%s", p->game_name);
		session.state.team = p->attached_as_team;
		
		output("Attached to game %d!\n", gameid);
		print_prompt();
		
	} packet(PACKET_GENERAL_STRING) {
	
		// Force string end
		((char *)payload)[plen - 1] = '\0';
		output("%s\n", (char *)payload);
		print_prompt();
	
	} else {
		output("Unknown packet!\n");
		packet_debug(ptype, plen, payload);
		print_prompt();
	}
}

void cb_remote_new_game_autoconnect(int ptype, int plen, void *payload) {
	reactor.callback_remote = NULL;
	
	if(ptype != PACKET_GAME_CREATION_RESPONSE) {
		fprintf(stderr, "Unexpected packet with type %d!\n", ptype);
		exit(EXIT_FAILURE);
	}
	
	if(plen != sizeof(packet_game_creation_response)) {
		fprintf(stderr, "Invalid packet size %d!\n", plen);
		exit(EXIT_FAILURE);
	}
	
	int gameid = ntohl(((packet_game_creation_response*)payload)->gameid);
	if(gameid <= 0) {
		output("Unable to create new game, error code is %d\n", gameid);
		return;
	}
	
	output("New game with ID %d was created, attaching...\n", gameid);
	send_game_attach_request(session.socket, gameid, TEAM_AUTO);
}

void cb_local_default(char *buff, int len) {
	tokenizer_init;	
	
	on command("help") {
		
		output("Commands list:\n");
		output("  ls               - get games list\n");
		output("  g_attach gameid  - attach to a game\n");
		output("  g_new [name]     - start a new game\n");
		output("  help             - this message\n");
		output("  shutdown         - shutdown the server (admin only)\n");
		output("  exit             - exit the app\n");
	
	} command("ls") {
		
		output("Getting games list...\n");
		send_games_list_request(session.socket);
		ommit_next_autoprompt++;
		
	} command("g_attach") {
		
		if(!tokget(cmd, 1)) {			
			output("Pass gameid as an argument!\n");
		} else {
			output("Attaching to game...\n");
			send_game_attach_request(session.socket, atoi(tokget(cmd, 1)), TEAM_AUTO);
			ommit_next_autoprompt++;
		}
		
	} command("g_new") {
		
		output("Creating new game...\n");
		reactor.callback_remote = &cb_remote_new_game_autoconnect;
		send_game_creation_request(session.socket, tokget(cmd, 1) ? tokget(cmd, 1) : session.login);
		ommit_next_autoprompt++;
		
	} command("shutdown") {
	
		output("Shutting the server down...\n");
		send_shutdown(session.socket);
		ommit_next_autoprompt++;
		
	} command("exit") {
	
		output("Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		if(tokget(cmd, 0))
			output("Unknown command \"%s\"! Try using \"help\".\n", tokget(cmd, 0));
	}
	
	tokenizer_free;
}
