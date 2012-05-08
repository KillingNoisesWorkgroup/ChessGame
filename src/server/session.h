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

void* Session(void *arg);
void create_session(int client_socket, struct sockaddr_in *client_addres);
void authentication(session *s, packet_auth_request *packet);
void send_auth_response(int dst, int val);
char* passw_to_hex(unsigned char * passw, int size);
void kick(session *s);

#endif
