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

void send_game_attach(int dst, uint32_t gameid, uint8_t team){
	packet_game_attach packet;
	game_description *g;
	packet.gameid = htonl(gameid);
	packet.attached_as_team = team;
	if(game_description_find(gameid, &g) == -1){
		packet_send(dst, PACKET_GAME_ATTACH, sizeof(packet), &packet);
		return;
	}
	strcpy(packet.game_name, g->name);
	packet_send(dst, PACKET_GAME_ATTACH, sizeof(packet), &packet);
}

void send_game_detach(int dst){
	packet_game_detach packet;
	packet_send(dst, PACKET_GAME_DETACH, sizeof(packet), &packet);
}

void send_games_list_response(int dst){
	char *games_list;
	char tmp[GAME_NAME_MAXSIZE + 10 + 1 + 1];
	int i, games_list_size;
	
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	games_list_size = (sizeof tmp)*(current_lobby.games->size) + 1;
	if((games_list = malloc(games_list_size)) == NULL){
		perror("malloc");
		exit(1);
	}
	games_list[0] = '\0';
	
	for(i = 0; i < current_lobby.games->size; i++){
		sprintf(tmp, "%10d %-*s\n",
			((game_description*)current_lobby.games->data[i])->id, GAME_NAME_MAXSIZE,
			((game_description*)current_lobby.games->data[i])->name);
		strcat(games_list, tmp);
	}
	games_list[games_list_size-1] = 0;
	
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	packet_send(dst, PACKET_GENERAL_STRING, games_list_size, games_list);
	free(games_list);
}

void send_users_list(int dst, packet_users_list_request *packet){
	char *users_list;
	char tmp[PLAYER_NAME_MAXSIZE + 10 + 1 + 1];
	int i, users_list_size;
	session *s;
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	users_list_size = (sizeof tmp)*(current_lobby.logins->size) + 1;
	if((users_list = malloc(users_list_size)) == NULL){
		perror("malloc");
		exit(1);
	}
	users_list[0] = '\0';
	
	for(i = 0; i < current_lobby.logins->size; i++){
		if(!(packet->online_only) || (session_find_login((login_entry*)current_lobby.logins->data[i], &s) != -1)){
			sprintf(tmp, "%10d %-*s\n",
				((login_entry*)current_lobby.logins->data[i])->id, 64,
				((login_entry*)current_lobby.logins->data[i])->login);
			strcat(users_list, tmp);
		}
	}
	users_list[users_list_size-1] = 0;
	
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	packet_send(dst, PACKET_GENERAL_STRING, users_list_size, users_list);
	free(users_list);
}

void send_game_desk(int dst, game_description* g){
	packet_game_desk packet;
	packet.desk = g->desk;
	packet_send(dst, PACKET_GAME_DESK, sizeof(desk_t), &packet);
}

void destroy_session(session* s){
	session *tmp;
	int pos;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	close(s->client_socket);
	
	pos = session_find_login(s->player, &tmp);
	dynamic_array_delete_at(current_lobby.sessions, pos);
	print_log(s->thread_info, "Session terminated");
	
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	
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

int session_find_login(login_entry *login, session **s){
	session* sess;
	int i, b = 0;
	if(login == NULL) return -1;
	sess = NULL;
	for(i = 0; i < current_lobby.sessions->size; i++){
		if(((session*)(current_lobby.sessions->data[i]))->player->id == login->id){
			sess = (session*)(current_lobby.sessions->data[i]);
			b = 1;
			break;
		}
	}
	*s = sess;
	if(b) return i;
	else return -1;
}

int authentication(session *s, packet_auth_request *packet){
	login_entry *login;
	char *hex;
	int userid, b = 1;
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	hex = passw_to_hex(packet->passw, ENCRYPTED_PASSWORD_LENGTH);
	
	if( (login_entry_find(packet->login, &login)) == -1){
		login = reg_new_user(packet, last_login_id++, hex);
		
		pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
		s->state = SESSION_STATE_LOBBY;
		s->player = login;
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		print_log(s->thread_info, "User %s(%d) was registrated", login->login, login->id);
	} else {
		if( strcmp(login->passw, hex) == 0){
			print_log(s->thread_info, "User %s(%d) successfully authenticated", login->login, login->id);
			
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			s->state = SESSION_STATE_LOBBY;
			s->player = login;
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		} else {
			print_log(s->thread_info, "Authentication of user %s(%d) failed", login->login, login->id);
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
	
	strcpy(name, packet->name);
	
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	
	game_d = init_game_description(last_game_id++);
	strcpy(game_d->name, packet->name);
	dynamic_array_add(current_lobby.games, game_d);
	
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	
	print_log(s->thread_info, "Game %s(%d) was created", game_d->name, game_d->id);
	
	return game_d->id;
}

void attach_to_game(session *s, uint32_t* gameid, uint8_t* team){
	game_description* g;
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	if((game_description_find(*gameid, &g) == -1) || (s->game != NULL)){
		send_game_attach(s->client_socket, 0, 0);
		pthread_mutex_unlock(&current_lobby.games->locking_mutex);
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		return;
	}
	s->game = g;
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	switch(*team){
	case TEAM_AUTO:
		if(g->white == NULL){
			g->white = s->player;
			*team = TEAM_WHITE;
			s->state = SESSION_STATE_PLAYING_WHITE;
			break;
		}
		if(g->black == NULL){
			g->black = s->player;
			*team = TEAM_BLACK;
			s->state = SESSION_STATE_PLAYING_BLACK;
		}
		else{
			pthread_mutex_lock(&g->spectators->locking_mutex);
			dynamic_array_add(g->spectators, s->player);
			pthread_mutex_unlock(&g->spectators->locking_mutex);
			*team = TEAM_SPECTATORS;
			s->state = SESSION_STATE_WATCHING_GAME;
		}
		break;
	case TEAM_WHITE:
		g->white = s->player;
		s->state = SESSION_STATE_PLAYING_WHITE;
		break;
	case TEAM_BLACK:
		g->black = s->player;
		s->state = SESSION_STATE_PLAYING_BLACK;
		break;
	case TEAM_SPECTATORS:
		pthread_mutex_lock(&g->spectators->locking_mutex);
		dynamic_array_add(g->spectators, s->player);
		pthread_mutex_unlock(&g->spectators->locking_mutex);
		s->state = SESSION_STATE_WATCHING_GAME;
		break;
	}
	
	print_log(s->thread_info, "Attached to game %s(%d)", g->name, g->id);
	send_game_attach(s->client_socket, *gameid, *team);
	send_game_desk(s->client_socket, g);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
}

void detach_from_game(session *s){
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	if(s->game == NULL){
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		return;
	}
	switch(s->state){
	case SESSION_STATE_PLAYING_WHITE:
		s->game->white = NULL;
		break;
	case SESSION_STATE_PLAYING_BLACK:
		s->game->black = NULL;
		break;
	}
	s->state = SESSION_STATE_LOBBY;
	print_log(s->thread_info, "Detached from game %s(%d)", s->game->name, s->game->id);
	s->game = NULL;
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
}

void figure_movement(session *s, packet_figure_move *packet){
	int i;
	session *target;
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	move_fig(&s->game->desk, packet->from_letter, packet->from_number, packet->to_letter, packet->to_number);
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	pthread_mutex_lock(&s->game->spectators->locking_mutex);
	
	if(session_find_login(s->game->white, &target) != -1){
		send_game_desk(target->client_socket, s->game);
	}
	if(session_find_login(s->game->black, &target) != -1){
		send_game_desk(target->client_socket, s->game);
	}
	for(i = 0; i < s->game->spectators->size; i++){
		if(session_find_login((login_entry*)s->game->spectators->data[i], &target) != -1){
			send_game_desk(target->client_socket, s->game);
		}
	}
	
	pthread_mutex_unlock(&s->game->spectators->locking_mutex);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
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
			detach_from_game(current_session);
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
			if(current_session->state != SESSION_STATE_LOBBY || !isadmin(current_session->player)){
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
			print_log(current_session->thread_info, "Got game attach request");
			attach_to_game(current_session, &gameid, &team);
			break;
		case PACKET_GAMES_LIST_REQUEST:
			print_log(current_session->thread_info, "Got games list request");
			send_games_list_response(current_session->client_socket);
			break;
		case PACKET_GAME_DETACH_REQUEST:
			print_log(current_session->thread_info, "Got game detach request");
			detach_from_game(current_session);
			send_game_detach(current_session->client_socket);
			break;
		case PACKET_FIGURE_MOVE:
			print_log(current_session->thread_info, "Got movement request packet");
			figure_movement(current_session, data);
			break;
		case PACKET_USERS_LIST_REQUEST:
			print_log(current_session->thread_info, "Got %s users list request",
				((packet_users_list_request*)data)->online_only ? "online" : "all");
			send_users_list(current_session->client_socket, data);
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
	new_session->game = NULL;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	dynamic_array_add(current_lobby.sessions, new_session);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	
	if( (pthread_create(&new_session->thread, NULL, (void*)Session, (void*)new_session)) != 0){
		perror("pthread_create");
		exit(1);
	}
}
