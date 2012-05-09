#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

#include "session.h"
#include "lobby.h"
#include "../shared/networking.h"
#include "memory_dump.h"
#include "shutdown_server.h"
#include "logging.h"

char* passw_to_hex(unsigned char * passw, int size){
	char* hex;
	char tmp[10];
	int i;
	if( (hex = malloc(size * 2 + 1)) == NULL){
		perror("malloc");
		exit(1);
	}
	hex[0] = '\0';
	for(i = 0; i < size; i++) {
		sprintf(tmp, "%02x", passw[i]);
		strcat(hex, tmp);
	}
	return hex;
}

void send_auth_response(int dst, int val){
	packet_auth_response packet;
	packet.response = (uint8_t)val;
	packet_send(dst, (packet_type_t)PACKET_AUTH_RESPONSE, (packet_length_t)sizeof(packet), &packet);
}

void destroy_session(session* s){
	//free(s->client_addres);
	close(s->client_socket);
	print_log(s->thread_info, "Session terminated");
	pthread_exit(NULL);
}

login_entry* reg_new_user(packet_auth_request* packet, int id, char* hex){
	login_entry* login;
	login = init_login_entry(id);
	strcpy(login->login, packet->login);
	strncpy(login->passw, hex, strlen(hex));
	dynamic_array_add(current_lobby.logins, login);
	create_memory_dump();
	return login;
}

void authentication(session *s, packet_auth_request *packet){
	login_entry *login;
	int last_id;
	char *hex;
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	if( current_lobby.logins->size == 0) last_id = 0;
	else {
		last_id = ((login_entry*)((current_lobby.logins)->data[current_lobby.logins->size - 1]))->id;
	}
	
	hex = passw_to_hex(packet->passw, ENCRYPTED_PASSWORD_LENGTH);
	
	if( (login_entry_find(packet->login, &login)) == -1){
		print_log(s->thread_info, "Authentication success with new user registration");
		login = reg_new_user(packet, last_id+1, hex);
		send_auth_response(s->client_socket, 1);
		
		pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
		s->state = SESSION_STATE_WORK;
		s->player = login;
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	} else {
		if( strcmp(login->passw, hex) == 0){
			print_log(s->thread_info, "Authentication success");
			send_auth_response(s->client_socket, 1);
			
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			s->state = SESSION_STATE_WORK;
			s->player = login;
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		} else {
			print_log(s->thread_info, "Authentication failure");
			send_auth_response(s->client_socket, 0);
			free(hex);
			pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
			destroy_session(s);
		}
	}
	free(hex);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
}

void* Session(void *arg){
	packet_type_t packet_type;
	packet_length_t length;
	session *current_session;
	void *data;
	
	current_session = arg;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	snprintf(current_session->thread_info, sizeof current_session->thread_info, "session %0lX", current_session->thread);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	print_log(current_session->thread_info, "Created new thread");
	
	while(1){
		if( !packet_recv(current_session->client_socket, &packet_type, &length, &data)){
			print_log(current_session->thread_info, "Client disconnected");
			destroy_session(current_session);
		}
		//packet_debug_full(packet_type, length, data);
		
		switch(packet_type){
		case PACKET_AUTH_REQUEST:
			authentication(current_session, data);
			break;
		case PACKET_SERVER_SHUTDOWN:
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			if(current_session->state != SESSION_STATE_WORK || !isadmin(current_session->player)){
					print_log(current_session->thread_info, "Got server shutdown packet, admin authentication error");
					pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
					destroy_session(current_session);
			} else {
				print_log(current_session->thread_info, "Got server shutdown packet");
				pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
				shutdown_server();
			}
			break;
		}
		free(data);
	}
}

void create_session(int client_socket, struct sockaddr_in *client_addres){
	session *new_session;
	if((new_session = malloc(sizeof(session))) == NULL){
		perror("malloc");
		exit(1);
	}
	new_session->id = current_lobby.sessions->size;
	new_session->state = SESSION_STATE_WAITING_FOR_AUTHENTICATION;
	new_session->client_socket = client_socket;
	new_session->client_addres = client_addres;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	dynamic_array_add(current_lobby.sessions, new_session);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	
	if( (pthread_create(&new_session->thread, NULL, (void*)Session, (void*)new_session)) != 0){
		perror("pthread_create");
		exit(1);
	}
}
