#ifndef H_SERVER_SESSION_GUARD
#define H_SERVER_SESSION_GUARD

#include <pthread.h>

#include "login_entry.h"
#include "../shared/networking.h"

#define SESSION_STATE_INITIAL_STATE 0
#define SESSION_STATE_WAITING_FOR_AUTHENTICATION 1

typedef struct session{
	int id;
	login_entry *player;
	int state;
	struct sockaddr *client_addres;
	int client_socket;
	pthread_t *thread;
} session;

void* Session(void *arg);
void create_session(int client_socket, struct sockaddr *client_addres);
void authentication(int client_socket, packet_auth_request *packet);

#endif

