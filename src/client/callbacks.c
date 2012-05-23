#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // for htonl() and etc

#include "client.h"
#include "helpers.h"
#include "callbacks.h"
#include "packet_helpers.h"

#include "../shared/helpers.h"

// Some helpers to fancy up the code
#define on if(0){}
#define command(tested_command) else if(strcmp(tokget(cmd, 0), (tested_command)) == 0)
#define packet(type) else if(ptype == type)
#define tokenizer_init tokenized_string_t cmd = tokenize_string(buff);
#define tokenizer_free free(cmd.str);

// Variable names must be all the same as in the prototype for 
// fancy helpers to work properly

// Callbacks definitions

void cb_remote_ingame(int ptype, int plen, void *payload);
void cb_remote_default(int ptype, int plen, void *payload);
void cb_remote_new_game_autoconnect(int ptype, int plen, void *payload);

void cb_local_ingame(char *buff, int len);
void cb_local_default(char *buff, int len);

// Implementation

void cb_remote_ingame(int ptype, int plen, void *payload) {
	on packet(PACKET_GAME_DESK) {

		packet_game_desk * p = payload;
		print_desk(p->desk);
		
	} packet(PACKET_GAME_DETACH) {
		
		reactor.callback_remote = NULL;
		reactor.callback_local = NULL;
		session.state.current = GAMESTATE_LOBBY;
		
		output(L"You are now in lobby!\n");
		print_prompt();
		
	} packet(PACKET_GENERAL_STRING) {
	
		// Force string end
		((char *)payload)[plen - 1] = '\0';
		output(L"%s\n", (char *)payload);
		print_prompt();
	
	} else {
		output(L"Unknown packet (ingame)!\n");
		packet_debug(ptype, plen, payload);
		print_prompt();
	}
}

void cb_remote_default(int ptype, int plen, void *payload) {
	on packet(PACKET_GAME_ATTACH) {
	
		packet_game_attach * p = payload;
		int gameid = ntohl(p->gameid);
		
		reactor.callback_remote = &cb_remote_ingame;
		reactor.callback_local = &cb_local_ingame;
		session.state.current = GAMESTATE_INGAME;
		snprintf(session.state.game_name, sizeof session.state.game_name, "%s", p->game_name);
		session.state.team = p->attached_as_team;
		session.state.desk_inverted = session.state.team == TEAM_BLACK;
		
		output(L"Attached to game %d!\n", gameid);
		print_prompt();
		
	} packet(PACKET_GENERAL_STRING) {
	
		// Force string end
		((char *)payload)[plen - 1] = '\0';
		output(L"%s\n", (char *)payload);
		print_prompt();
	
	} else {
		output(L"Unknown packet!\n");
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
		output(L"Unable to create new game, error code is %d\n", gameid);
		return;
	}
	
	output(L"New game with ID %d was created, attaching...\n", gameid);
	send_game_attach_request(session.socket, gameid, TEAM_AUTO);
}

void cb_local_ingame(char *buff, int len) {
	tokenizer_init;
	
	on command("help") {
		
		output(L"Commands list:\n");
		output(L"  I from to  - move figure from <from> position to <to>\n");
		output(L"  msg text   - send a chat message\n");
		output(L"  detach     - quit current game session\n");
		output(L"  help       - this message\n");
		output(L"  exit       - exit the app\n");
	
	} command("I") {
		
		if(!tokget(cmd, 1) || !tokget(cmd, 2) || tokget(cmd, 3) || strlen(tokget(cmd, 1)) != 2 || strlen(tokget(cmd, 2)) != 2) {
			output(L"Valid format for this command is \"I from to\" like \"I e2 e4\".\n");
		} else {
		
			int from_letter = char_to_num(tokget(cmd, 1)[0]) - 1;
			int to_letter   = char_to_num(tokget(cmd, 2)[0]) - 1;
			
			int from_number = atoi(&tokget(cmd, 1)[1]) - 1;
			int to_number   = atoi(&tokget(cmd, 2)[1]) - 1;
			
			// Input validation
			if(from_letter < 0 || to_letter < 0 || from_number < 0 || to_number < 0 || 
				from_letter > 7 || to_letter > 7 || from_number > 7 || to_number > 7 ) {
				output(L"Invalid arguments! Use this command like \"I e2 e4\"!\n");
			} else {
				send_figure_move(session.socket, from_letter, from_number, to_letter, to_number);
				ommit_next_autoprompt++;
			}
		}
		
	} command("msg") {
		
		if(tokget(cmd, 1))
			send_chat_message_outgoing(session.socket, tokget(cmd, 1));
		else
			send_chat_message_outgoing(session.socket, "");
		
	} command("detach") {
		
		output(L"Detaching a game...\n");
		send_game_detach_request(session.socket);
		ommit_next_autoprompt++;
		
	} command("exit") {
	
		output(L"Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		if(tokget(cmd, 0) && strlen(tokget(cmd, 0)) > 0)
			output(L"Unknown command \"%s\"! Try using \"help\".\n", tokget(cmd, 0));
	}
	
	tokenizer_free;
}

void cb_local_default(char *buff, int len) {
	tokenizer_init;	
	
	on command("help") {
		
		output(L"Commands list:\n");
		output(L"  g_online                - get online games list\n");
		output(L"  g_attach gameid [team]  - attach to a game\n");
		output(L"  g_new [name]            - start a new game\n");
		output(L"  g_rand                  - join a random game\n");
		output(L"  g_all                   - get a history of games\n");
		output(L"  g_log gameid            - get game log\n");
		output(L"  g_del gameid            - delete a game (admin only)\n");
		output(L"  p_online                - get online players list\n");
		output(L"  p_list                  - get all players list\n");
		output(L"  p_rating [userid]       - get player(s) rating\n");
		output(L"  kick userid gameid      - kick player from game (admin only)\n");
		output(L"  p_del userid            - delete user (admin only)\n");
		output(L"  shutdown                - shutdown the server (admin only)\n");
		output(L"  help                    - this message\n");
		output(L"  exit                    - exit the app\n");
	
	} command("p_online") {
		
		output(L"Getting online players list...\n");
		send_users_list_request(session.socket, 1);
		ommit_next_autoprompt++;
		
	} command("p_list") {
		
		output(L"Getting all players list...\n");
		send_users_list_request(session.socket, 0);
		ommit_next_autoprompt++;
		
	} command("p_rating") {
		
		uint32_t userid = 0;
		if(tokget(cmd, 1)) userid = atoi(tokget(cmd, 1));
		if(userid)
		  output(L"Getting player #%d rating...\n", userid);
		else
		  output(L"Getting players rating...\n");		
		send_user_rating_request(session.socket, userid);
		ommit_next_autoprompt++;
		
	} command("g_online") {
		
		output(L"Getting online games list...\n");
		send_games_list_request(session.socket);
		ommit_next_autoprompt++;
		
	} command("g_all") {
		
		output(L"Getting games history...\n");
		send_games_history_request(session.socket);
		ommit_next_autoprompt++;
		
	} command("g_attach") {
		
		if(!tokget(cmd, 1)) {			
			output(L"Pass gameid as an argument!\n");
		} else {
			uint8_t team = TEAM_AUTO;
			if(tokget(cmd, 2)) {
				switch(tokget(cmd, 2)[0]) {
					case 'w':
					case 'W': team = TEAM_WHITE; break;
					case 'b': 
					case 'B': team = TEAM_BLACK; break;
					case 's': 
					case 'S': team = TEAM_SPECTATORS; break;
				}
			}			
			
			output(L"Attaching to game...\n");
			send_game_attach_request(session.socket, atoi(tokget(cmd, 1)), team);
			ommit_next_autoprompt++;
		}
		
	} command("g_new") {
		
		output(L"Creating new game...\n");
		reactor.callback_remote = &cb_remote_new_game_autoconnect;
		send_game_creation_request(session.socket, tokget(cmd, 1) ? tokget(cmd, 1) : session.login);
		ommit_next_autoprompt++;
		
	} command("g_rand") {
		
		output(L"Entering matchmaking queue, you will be autoattached when ready...\n");
		send_matchmaking_queue_request(session.socket);
		ommit_next_autoprompt++;
		
	} command("g_log") {
		
		if(!tokget(cmd, 1)) {			
			output(L"Pass gameid as an argument!\n");
		} else {
			output(L"Requesting game log...\n");
			send_game_log_request(session.socket, atoi(tokget(cmd, 1)));
			ommit_next_autoprompt++;
		}
		
	} command("g_del") {
		
		if(!tokget(cmd, 1)) {			
			output(L"Pass gameid as an argument!\n");
		} else {
			output(L"Deleting game...\n");
			send_game_delete_request(session.socket, atoi(tokget(cmd, 1)));
			ommit_next_autoprompt++;
		}
		
	} command("p_del") {
		
		if(!tokget(cmd, 1)) {			
			output(L"Pass userid as an argument!\n");
		} else {
			output(L"Deleting user...\n");
			send_user_delete_request(session.socket, atoi(tokget(cmd, 1)));
			ommit_next_autoprompt++;
		}
		
	} command("kick") {
		
		if(!tokget(cmd, 2)) {			
			output(L"Pass userid and gameid as an argument!\n");
		} else {
			output(L"Kicking...\n");
			send_kick_from_game_request(session.socket, atoi(tokget(cmd, 2)), atoi(tokget(cmd, 1)));
			ommit_next_autoprompt++;
		}
		
	} command("shutdown") {
	
		output(L"Shutting the server down...\n");
		send_shutdown(session.socket);
		ommit_next_autoprompt++;
		
	} command("exit") {
	
		output(L"Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		if(tokget(cmd, 0) && strlen(tokget(cmd, 0)) > 0)
			output(L"Unknown command \"%s\"! Try using \"help\".\n", tokget(cmd, 0));
	}
	
	tokenizer_free;
}
