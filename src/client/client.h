#ifndef H_CLIENT_CLIENT_GUARD
#define H_CLIENT_CLIENT_GUARD

#include <pthread.h>

#include "gamestate.h"

#include "../shared/networking.h"

// Maximum length of one command input from stdin
#define MAX_INPUT_SIZE 1024

typedef struct server_rec {
	struct hostent *host;
	int port;
} server_rec;

typedef struct session_rec {
	server_rec server;

	char login[PLAYER_NAME_MAXSIZE];
	unsigned char password_encrypted[ENCRYPTED_PASSWORD_LENGTH];
	
	uint32_t userid;
	
	gamestate_rec state;
	
	int socket;
} session_rec;

typedef void (*callback_remote_t)(int ptype, int plen, void *payload);
typedef void (*callback_local_t)(char *buff, int len);

// This is NOT a real reactor.
typedef struct reactor_rec {
	pthread_t 			thread_input_remote;
	callback_remote_t 	callback_remote;
	
	pthread_t 			thread_input_local;
	callback_local_t 	callback_local;
	
	pthread_mutex_t locking_mutex;
} reactor_rec;

session_rec session;
reactor_rec reactor;

#endif
