#include <stdlib.h>
#include <stdio.h>

#include "session.h"
#include "lobby.h"
#include "../shared/networking.h"

void authentication(packet_auth_request *packet){
	login_entry *login;
	int *last_id;
	last_id = &(((login_entry*)((current_lobby.logins)->data[current_lobby.logins->size]))->id);
	login = init_login_entry(*last_id);
	*last_id +=1;
}

void* Session(void *arg){
	int *packet_type, *length, session_id;
	void *data;
	printf("Trololo, i'm a thread, and this is jackass\n");
	session_id = *(int*)arg;
	while(1){
		packet_recv(session_id, packet_type, length, &data);
		switch(*packet_type){
		case PACKET_AUTH_REQUEST:
			authentication(data);
			break;
		}
	}
}

void create_session(int client_socket, struct sockaddr *client_addres){
	session *new_session;
	pthread_t *thread;
	if( (new_session = malloc(sizeof(session))) == NULL){
		perror("malloc");
		exit(1);
	}	
	if( (thread = malloc(sizeof(pthread_t))) == NULL){
		perror("malloc");
		exit(1);
	}
	new_session->id = current_lobby.sessions->size;
	new_session->state = SESSION_STATE_WAITING_FOR_AUTHENTICATION;
	new_session->client_socket = client_socket;
	new_session->client_addres = client_addres;
	new_session->thread = thread;
	
	dynamic_array_add(current_lobby.sessions, new_session);
	if( (pthread_create(thread, NULL, (void*)Session, (void*)&(new_session->id))) != 0){
		perror("pthread_create");
		exit(1);
	}
}

