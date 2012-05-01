#ifndef H_CLIENT_CLIENT_GUARD
#define H_CLIENT_CLIENT_GUARD

#include <pthread.h>

#include "../shared/shared.h"
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
	
	int socket;
} session_rec;

// This is NOT a real reactor.
typedef struct reactor_rec {
	pthread_t thread_input_remote;
	pthread_t thread_input_local;
	pthread_mutex_t locking_mutex;
} reactor_rec;

session_rec session;
reactor_rec reactor;

#endif