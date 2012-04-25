#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

#include "session.h"
#include "lobby.h"
#include "../shared/networking.h"

void authentication(int client_socket, packet_auth_request *packet){
	login_entry *login;
	int *last_id;
	// last user id
	last_id = &(((login_entry*)((current_lobby.logins)->data[current_lobby.logins->size]))->id);
	
	if( (login = login_entry_find(packet->login)) == NULL){
		*last_id +=1;
		login = init_login_entry(*last_id);
		strncpy(login->login, packet->login, PLAYER_NAME_MAXSIZE);
		memcpy(login->passw, packet->passw, ENCRYPTED_PASSWORD_LENGTH);
		create_password(*last_id, packet->login, packet->passw);
		dynamic_array_add(current_lobby.logins, login);
	} else {
		if( strncmp(login->passw, packet->passw, ENCRYPTED_PASSWORD_LENGTH) == 0){
			// packet_send(client_socket, )
		} else {
			shutdown(client_socket, SHUT_RDWR);
			pthread_exit(NULL);
		}
	}
	free(packet->login);
	free(packet->passw);
	free(packet);
}

void* Session(void *arg){
	packet_type_t packet_type;
	packet_length_t length;
	session *current_session;
	void *data;
	
	printf("Trololo, i'm a thread, and this is jackass\n");
	current_session = arg;
	
	while(1){
		packet_recv(current_session->client_socket, &packet_type, &length, &data);
		switch(packet_type){
			case PACKET_AUTH_REQUEST:
				authentication(current_session->client_socket, data);
				break;
		}
	}
}

void create_session(int client_socket, struct sockaddr *client_addres){
	session *new_session;
	pthread_t thread;
	if( (new_session = malloc(sizeof(session))) == NULL){
		perror("malloc");
		exit(1);
	}
	new_session->id = current_lobby.sessions->size;
	new_session->state = SESSION_STATE_WAITING_FOR_AUTHENTICATION;
	new_session->client_socket = client_socket;
	new_session->client_addres = client_addres;
	new_session->thread = thread;
	
	dynamic_array_add(current_lobby.sessions, new_session);
	if( (pthread_create(&thread, NULL, (void*)Session, (void*)new_session)) != 0){
		perror("pthread_create");
		exit(1);
	}
}

