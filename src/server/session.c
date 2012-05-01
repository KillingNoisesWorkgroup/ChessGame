#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

#include "session.h"
#include "lobby.h"
#include "../shared/networking.h"
#include "memory_dump.h"

char* passw_to_hex(char* passw, int size){
	char* hex;
	int i;
	if( (hex = malloc(size * 2 + 1)) == NULL){
		perror("malloc");
		exit(1);
	}
	for( i = 0; i < size; i++) sprintf(&(hex[i*2]), "%02x", passw[i]);
	hex[i*2 + 1] = 0;
	return hex;
}

void send_auth_response(int dst, int val){
	packet_auth_response packet;
	packet.response = (uint8_t)val;
	packet_send(dst, (packet_type_t)PACKET_AUTH_RESPONSE, (packet_length_t)sizeof(packet), &packet);
}

void authentication(int client_socket, packet_auth_request *packet){
	login_entry *login;
	int last_id;
	char *hex;
	// last user id
	printf("trying to authenticate %s\n", packet->login);
	if( current_lobby.logins->size == 0) last_id = 0;
	else {
		last_id = 
		((login_entry*)((current_lobby.logins)->data[current_lobby.logins->size - 1]))->id;
	}
	hex = passw_to_hex(packet->passw, strlen(packet->passw));
	if( (login = login_entry_find(packet->login)) == NULL){
		printf("it's a new user! lets make him/her a registration\n");
		fflush(stdout);
		login = init_login_entry(last_id+1);
		strncpy(login->login, packet->login, strlen(packet->login));
		strncpy(login->passw, hex, strlen(hex));
		dynamic_array_add(current_lobby.logins, login);
		send_auth_response(client_socket, 1);
		create_memory_dump();
	} else {
		if( strcmp(login->passw, hex) == 0){
			printf("password is correct\n");
			send_auth_response(client_socket, 1);
		} else {
			printf("password is incorrect, closing socket and exiting thread\n");
			send_auth_response(client_socket, 0);
			close(client_socket);
			pthread_exit(NULL);
		}
	}
	free(hex);
}

void* Session(void *arg){
	packet_type_t packet_type;
	packet_length_t length;
	session *current_session;
	void *data;
	
	current_session = arg;
	printf("creating a thread for session for client with socket %d\n", current_session->client_socket);
	
	while(1){
		if( !packet_recv(current_session->client_socket, &packet_type, &length, &data)){
			printf("client with socket %d disconnected\n", current_session->client_socket);
			break;
		}
		
		packet_debug(packet_type, length, data);
		
		switch(packet_type){
			case PACKET_AUTH_REQUEST:
				authentication(current_session->client_socket, data);
				break;
		}
	}
}

void create_session(int client_socket, struct sockaddr_in *client_addres){
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

