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
#include "game_description.h"

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
	packet.userid = htonl((uint32_t)val);
	packet_send(dst, PACKET_AUTH_RESPONSE, sizeof(packet), &packet);
}

void send_game_creation_response(int dst, int val){
	packet_game_creation_response packet;
	packet.gameid = htonl((uint32_t)val);
	packet_send(dst, PACKET_GAME_CREATION_RESPONSE, sizeof(packet), &packet);
}

void send_game_attach_response(int dst, uint32_t gameid, uint8_t team){
	packet_game_attach packet;
	packet.gameid = htonl(gameid);
	packet.attached_as_team = team;
	packet_send(dst, PACKET_GAME_ATTACH, sizeof(packet), &packet);
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
	return login;
}

int authentication(session *s, packet_auth_request *packet){
	login_entry *login;
	char *hex;
	int userid, b = 1;
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	if( current_lobby.logins->size == 0) last_login_id = 1;
	
	hex = passw_to_hex(packet->passw, ENCRYPTED_PASSWORD_LENGTH);
	
	if( (login_entry_find(packet->login, &login)) == -1){
		login = reg_new_user(packet, last_login_id++, hex);
		
		pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
		s->state = SESSION_STATE_WORK;
		s->player = login;
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		print_log(s->thread_info, "User %s(%d) was registrated", login->login, login->id);
	} else {
		if( strcmp(login->passw, hex) == 0){
			print_log(s->thread_info, "User %s(%d) successfully authenticated", login->login, login->id);
			
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			s->state = SESSION_STATE_WORK;
			s->player = login;
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		} else {
			print_log(s->thread_info, "Authentication of user %s(%d) failed", login->login, login->id);
			free(hex);
			pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
			b = 0;
		}
	}
	free(hex);
	userid = login->id;
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	if(b) return userid;
	else return -1;
}

int create_game(session* s, packet_game_creation_request *packet){
	game_description* game_d;
	char* name;
	
	if((name = malloc(GAME_NAME_MAXSIZE)) == NULL){
		perror("malloc");
		exit(1);
	}
	
	strcpy(name, packet->name);
	if(current_lobby.games->size == 0){
		last_game_id = 1;
	}
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	
	game_d = init_game_description(last_game_id++);
	dynamic_array_add(current_lobby.games, game_d);
	
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	
	print_log(s->thread_info, "Game with id %d was created", game_d->id);
	
	return game_d->id;
}

int attach_to_game(session *s, uint32_t* gameid, uint8_t* team){
	game_description* g;
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	
	if(game_description_find(*gameid, &g) == -1){
		return -1;
	}
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	switch(*team){
	case TEAM_AUTO:
		if(g->white == NULL){
			g->white = s->player;
			*team = TEAM_WHITE;
			break;
		}
		if(g->black == NULL){
			g->black = s->player;
			*team = TEAM_BLACK;
		}
		else{
			dynamic_array_add(g->spectators, (void*)s->player);
			*team = TEAM_SPECTATORS;
		}
		break;
	case TEAM_WHITE:
		g->white = s->player;
		break;
	case TEAM_BLACK:
		g->black = s->player;
		break;
	case TEAM_SPECTATORS:
		dynamic_array_add(g->spectators, (void*)s->player);
		break;
	}
	
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	return 1;
}

void* Session(void *arg){
	packet_type_t packet_type;
	packet_length_t length;
	session *current_session;
	uint32_t gameid;
	uint8_t team;
	int retval;
	void *data;
	
	current_session = arg;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	snprintf(current_session->thread_info, sizeof current_session->thread_info, "session %0lX", current_session->thread);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	print_log("main", "Created %s thread", current_session->thread_info);
	
	while(1){
		if( !packet_recv(current_session->client_socket, &packet_type, &length, &data)){
			print_log(current_session->thread_info, "Client disconnected");
			destroy_session(current_session);
		}
		//packet_debug_full(packet_type, length, data);
		
		switch(packet_type){
		case PACKET_AUTH_REQUEST:
			retval = authentication(current_session, data);
			send_auth_response(current_session->client_socket, retval);
			if(retval == -1) destroy_session(current_session);
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
		case PACKET_GAME_CREATION_REQUEST:
			print_log(current_session->thread_info, "Got game creation packet");
			send_game_creation_response(current_session->client_socket, create_game(current_session, data));
			break;
		case PACKET_GAME_ATTACH_REQUEST:
			gameid = ntohl(((packet_game_attach_request*)data)->gameid);
			team = ((packet_game_attach_request*)data)->team;
			print_log(current_session->thread_info, "Got game attach packet gameid = %d, team = %d", gameid, team);
			attach_to_game(current_session, &gameid, &team);
			send_game_attach_response(current_session->client_socket, gameid, team);
			break;
		default:
			print_log(current_session->thread_info, "Got unknown packet(%d)", packet_type);
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
