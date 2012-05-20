#ifndef H_SERVER_SESSION_GUARD
#define H_SERVER_SESSION_GUARD

#include <pthread.h>

#include "login_entry.h"
#include "../shared/networking.h"

#define SESSION_STATE_INITIAL_STATE 0
#define SESSION_STATE_WAITING_FOR_AUTHENTICATION 1
#define SESSION_STATE_WORK 2

typedef struct session{
	int id;
	login_entry *player;
	int state;
	struct sockaddr_in *client_addres;
	int client_socket;
	pthread_t thread;
	char thread_info[128];
} session;

// Initializes new session
void create_session(int client_socket, struct sockaddr_in *client_addres);

// Does all the free's after session is finished
void destroy_session(session *s);

// Thread function, where all of the packets processing take place
void* Session(void *arg);

// Registers new user
login_entry* reg_new_user(packet_auth_request* packet, int id, char* hex);

// Authenticates user. Returns user id on success, -1 on failure
int authentication(session *s, packet_auth_request *packet);

// Converts passw to string, containing its hexadecimal representation
char* passw_to_hex(unsigned char * passw, int size);

// Creates new game. Returns game id
int create_game(session* s, packet_game_creation_request *packet);

// Attaches user to game. Returns 1 on success, -1 on failure
int attach_to_game(session *s, uint32_t* gameid, uint8_t* team);


// Sends packet_auth_response
void send_auth_response(int dst, int val);

// Sends packet_game_creation response
void send_game_creation_response(int dst, int val);

// Sends packet_game_attach_response. If attaching to game is failed, gameid should be -1
void send_game_attach_response(int dst, uint32_t gameid, uint8_t team);

// Sends string, containing list of all games
void send_games_list_response(int dst);

#endif
