#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h> // for htonl() and etc

#include "client.h"
#include "helpers.h"
#include "callbacks.h"
#include "packet_helpers.h"

// Some helpers to fancy up the code
#define on if(0){}
#define command(cmd) else if(check_command(buff, (cmd)))
#define packet(type) else if(ptype == type)

// Variable names must be all the same as in the prototype for 
// fancy helpers to work properly

// Default callbacks

void cb_remote_default(int ptype, int plen, void *payload) {
	on else {
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
	if(gameid < 0) {
		output("Unable to create new game, error code is %d\n", gameid);
		return;
	}
	
	output("New game with ID %d was created, attaching...\n", gameid);
	send_game_attach_request(session.socket, gameid, TEAM_AUTO);
}

void cb_local_default(char *buff, int len) {	
	on command("help") {
		
		output("Commands list:\n");
		output("  g_new\n");
		output("  help\n");
		output("  shutdown\n");
		output("  exit\n");
	
	} command("g_new") {
		
		output("Creating new game...\n");
		reactor.callback_remote = &cb_remote_new_game_autoconnect;
		send_game_creation_request(session.socket, session.login);
		
	} command("shutdown") {
	
		output("Shutting the server down...\n");
		send_shutdown(session.socket);
		
	} command("exit") {
	
		output("Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		output("Unknown command!\n");
	}
}
