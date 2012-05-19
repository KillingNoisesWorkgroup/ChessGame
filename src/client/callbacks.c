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

// Default callbacks

void cb_remote_default(int ptype, int plen, void *payload) {
	on packet(PACKET_GAME_ATTACH) {
		output("Server wanted us to attach to game %d!\n", ntohl(((packet_game_attach*)(payload))->gameid));
	} else {
		output("Unknown packet!\n");
		packet_debug(ptype, plen, payload);
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
		
	} command("g_attach") {
		
		if(!tokget(cmd, 1)) {			
			output("Pass gameid as an argument!\n");
		} else {
			output("Attaching to game...\n");
			send_game_attach_request(session.socket, atoi(tokget(cmd, 1)), TEAM_AUTO);
		}
		
	} command("g_new") {
		
		output("Creating new game...\n");
		reactor.callback_remote = &cb_remote_new_game_autoconnect;
		send_game_creation_request(session.socket, tokget(cmd, 1) ? tokget(cmd, 1) : session.login);
		
	} command("shutdown") {
	
		output("Shutting the server down...\n");
		send_shutdown(session.socket);
		
	} command("exit") {
	
		output("Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		if(tokget(cmd, 0))
			output("Unknown command \"%s\"! Try using \"help\".\n", tokget(cmd, 0));
	}
	
	tokenizer_free;
}
