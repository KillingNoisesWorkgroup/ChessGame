#ifndef H_SERVER_SESSION_GUARD
#define H_SERVER_SESSION_GUARD

#include <pthread.h>

#include "login_entry.h"
#include "../shared/networking.h"
#include "game_description.h"
#include "../shared/common.h"

#define SESSION_STATE_INITIAL_STATE 0
#define SESSION_STATE_WAITING_FOR_AUTHENTICATION 1
#define SESSION_STATE_LOBBY 2
#define SESSION_STATE_PLAYING_WHITE 3
#define SESSION_STATE_PLAYING_BLACK 4
#define SESSION_STATE_WATCHING_GAME 5

typedef struct session{
	int id;
	login_entry *player;
	int state;
	struct sockaddr_in *client_addres;
	int client_socket;
	pthread_t thread;
	game_description *game;
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

// Searches login entry login in sessions array and places it in *s. Returns 1 on success, 0 on failure, -1 on error
int session_find_login(login_entry *login, session **s);

// Authenticates user. Returns user id on success, -1 on failure
int authentication(session *s, packet_auth_request *packet);

// Converts passw to string, containing its hexadecimal representation
char* passw_to_hex(unsigned char * passw, int size);

// Creates new game. Returns game id
int create_game(session* s, packet_game_creation_request *packet);

// Attaches user to game if it can and sends corresponding packet_game_attach
void attach_to_game(session *s, uint32_t* gameid, uint8_t* team);

// Detaching user from game
void detach_from_game(session *s);


// Sends packet_auth_response
void send_auth_response(int dst, int val);

// Sends packet_game_creation response
void send_game_creation_response(int dst, int val);

// Sends packet_game_attach_response. If attaching to game is failed, gameid should be -1
void send_game_attach(int dst, uint32_t gameid, uint8_t team);

// Sends string, containing list of all games
void send_games_list_response(int dst);

// Sends packet_game_detach
void send_game_detach(int dst);

// Sends packet_game_desk
void send_game_desk(int dst, game_description* g);

#endif
