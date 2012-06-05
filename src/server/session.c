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
#include "game_log.h"

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

void send_games_history(int dst){
	char msg[128];
	sprintf(msg, "There was %d games so far. That's all i know...\n", last_game_id-1);
	packet_send(dst, PACKET_GENERAL_STRING, strlen(msg), msg);
}

void send_game_log(int dst, uint32_t id){
	FILE *game_log;
	game_description *game;
	char *msg;
	int size = 0, b = 0;
	if(game_description_find(id, &game) == -1){
		game_log = open_game_log(id);
		b = 1;
	} else {
		game_log = game->game_log;
		rewind(game_log);
	}
	size = getdelim(&msg, &size, 0, game_log);
	packet_send(dst, PACKET_GENERAL_STRING, size, msg);
	free(msg);
	if(b) fclose(game_log);
}

void send_game_desk(int dst, game_description* g){
	packet_game_desk packet;
	packet.desk = g->desk;
	packet_send(dst, PACKET_GAME_DESK, sizeof(desk_t), &packet);
}

void destroy_session(session* s){
	session *tmp;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	close(s->client_socket);
	
	dynamic_array_delete_at(current_lobby.sessions, s->id);
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
	fprintf(game_d->game_log, "Game name: %s\n", game_d->name);
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
	
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
	switch(*team){
	case TEAM_AUTO:
		if(g->white == s->player){
			*team = TEAM_WHITE;
			s->state = SESSION_STATE_PLAYING_WHITE;
		} else if(g->black == s->player){
			*team = TEAM_BLACK;
			s->state = SESSION_STATE_PLAYING_BLACK;
		} else if(g->white == NULL){
			g->white = s->player;
			*team = TEAM_WHITE;
			s->state = SESSION_STATE_PLAYING_WHITE;
		} else if(g->black == NULL){
			g->black = s->player;
			*team = TEAM_BLACK;
			s->state = SESSION_STATE_PLAYING_BLACK;
		} else {
			pthread_mutex_lock(&g->spectators->locking_mutex);
			dynamic_array_add(g->spectators, s->player);
			pthread_mutex_unlock(&g->spectators->locking_mutex);
			*team = TEAM_SPECTATORS;
			s->state = SESSION_STATE_WATCHING_GAME;
		}
		s->game = g;
		break;
	case TEAM_WHITE:
		if(g->white == NULL || g->white == s->player){
			g->white = s->player;
			s->state = SESSION_STATE_PLAYING_WHITE;
			s->game = g;
		} else {
			send_game_attach(s->client_socket, 0, 0);
			pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
			pthread_mutex_unlock(&current_lobby.games->locking_mutex);
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
			return;
		}
		break;
	case TEAM_BLACK:
		if(g->black == NULL || g->black == s->player){
			g->black = s->player;
			s->state = SESSION_STATE_PLAYING_WHITE;
			s->game = g;
		} else {
			send_game_attach(s->client_socket, 0, 0);
			pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
			pthread_mutex_unlock(&current_lobby.games->locking_mutex);
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
			return;
		}
		break;
	case TEAM_SPECTATORS:
		s->game = g;
		pthread_mutex_lock(&g->spectators->locking_mutex);
		dynamic_array_add(g->spectators, s->player);
		pthread_mutex_unlock(&g->spectators->locking_mutex);
		s->state = SESSION_STATE_WATCHING_GAME;
		break;
	}
	
	if(g->black && g->white) g->state = GAME_STATE_READY;
	
	print_log(s->thread_info, "Attached to game %s(%d)", g->name, g->id);
	send_game_attach(s->client_socket, *gameid, *team);
	send_game_desk(s->client_socket, g);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
}

void detach_from_game(session *s){
	if(s->game == NULL){
		pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
		return;
	}
	s->state = SESSION_STATE_LOBBY;
	print_log(s->thread_info, "Detached from game %s(%d)", s->game->name, s->game->id);
	s->game = NULL;
}

void detach_all(game_description *g){
	session *white, *black, *spectator;
	int i;
	if(session_find_login(g->white, &white) != -1){
		detach_from_game(white);
		send_game_detach(white->client_socket);
	}
	if(session_find_login(g->black, &black) != -1){
		detach_from_game(black);
		send_game_detach(black->client_socket);
	}
	for(i = 0; i < g->spectators->size; i++){
		if(session_find_login(((login_entry*)(g->spectators->data[i])), &spectator) != -1){
			detach_from_game(spectator);
			send_game_detach(spectator->client_socket);
		}
	}
}

int player_move(session *s){
	if(((s->state == SESSION_STATE_PLAYING_WHITE) && white_team_turn(s->game)) ||
		(s->state == SESSION_STATE_PLAYING_BLACK) && black_team_turn(s->game)){
		return 1;
	} else return 0;
}

void win(session *s){
	int i;
	char tmp[128];
	session *other;
	
	s->player->rating ++;
	
	if(s->state == SESSION_STATE_PLAYING_WHITE){
		snprintf(tmp, sizeof tmp, "You win!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		if(session_find_login(s->game->black, &other) != -1){
			snprintf(tmp, sizeof tmp, "You lose!\n");
			packet_send(other->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		}
		s->game->black->rating--;
	} else if(s->state == SESSION_STATE_PLAYING_BLACK){
		sprintf(tmp, "You win!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		if(session_find_login(s->game->white, &other) != -1){
			sprintf(tmp, "You lose!\n");
			packet_send(other->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		}
		s->game->black->rating --;
	}
	s->game->state = GAME_STATE_OVER;
	detach_all(s->game);
}

void surrender(session *s){
	int i;
	char tmp[128];
	session *other;
	
	s->player->rating --;
	if(s->state == SESSION_STATE_PLAYING_WHITE){
		sprintf(tmp, "You surrendered!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		if(session_find_login(s->game->black, &other) != -1){
			sprintf(tmp, "Your opponent surrendered! You win!\n");
			packet_send(other->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		}
		s->game->black->rating ++;
	} else if(s->state == SESSION_STATE_PLAYING_BLACK){
		sprintf(tmp, "You surrendered!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		if(session_find_login(s->game->white, &other) != -1){
			sprintf(tmp, "Your opponent surrendered! You win!\n");
			packet_send(other->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		}
		s->game->black->rating ++;
	}
	s->game->state = GAME_STATE_OVER;
	detach_all(s->game);
}

void figure_movement(session *s, packet_figure_move *packet){
	int i, dst_fig, dst_col, src_col;
	char tmp[128];
	session *target;
	pthread_mutex_lock(&current_lobby.games->locking_mutex);
	if(s->game->state != GAME_STATE_READY){
		sprintf(tmp, "Game is not ready!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		pthread_mutex_unlock(&current_lobby.games->locking_mutex);
		return;
	}
	if(!player_move(s)){
		sprintf(tmp, "That's not your turn!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		pthread_mutex_unlock(&current_lobby.games->locking_mutex);
		return;
	}
	
	src_col = get_fig_color(&s->game->desk, packet->from_letter, packet->from_number);
	
	dst_fig = get_fig_type(&s->game->desk, packet->to_letter, packet->to_number);
	dst_col = get_fig_color(&s->game->desk, packet->to_letter, packet->to_number);
	
	if((s->state == SESSION_STATE_PLAYING_WHITE && src_col != FIGURE_COLOR_WHITE) ||
		(s->state == SESSION_STATE_PLAYING_BLACK && src_col != FIGURE_COLOR_BLACK)){
		sprintf(tmp, "That's not your figure!\n");
		packet_send(s->client_socket, PACKET_GENERAL_STRING, strlen(tmp), tmp);
		pthread_mutex_unlock(&current_lobby.games->locking_mutex);
		return;
	}
	
	move_fig(&s->game->desk, packet->from_letter, packet->from_number, packet->to_letter, packet->to_number);
	game_log_move(s->game, packet);
	s->game->moves_made ++;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_lock(&current_lobby.logins->locking_mutex);
	
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
	
	if(dst_fig == FIGURE_KING){
		if(dst_col == FIGURE_COLOR_WHITE){
			if(s->state == SESSION_STATE_PLAYING_WHITE){
				surrender(s);
			} else if(s->state == SESSION_STATE_PLAYING_BLACK){
				win(s);
			}
		} else if(dst_col == FIGURE_COLOR_BLACK){
			if(s->state == SESSION_STATE_PLAYING_BLACK){
				surrender(s);
			} else if(s->state == SESSION_STATE_PLAYING_WHITE){
				win(s);
			}
		}
	}
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	pthread_mutex_unlock(&current_lobby.logins->locking_mutex);
	pthread_mutex_unlock(&current_lobby.games->locking_mutex);
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
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			detach_from_game(current_session);
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
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
			pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
			detach_from_game(current_session);
			pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
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
		case PACKET_GAMES_HISTORY_REQUEST:
			print_log(current_session->thread_info, "Got games history request");
			send_games_history(current_session->client_socket);
			break;
		case PACKET_GAME_LOG_REQUEST:
			print_log(current_session->thread_info, "Got game %d log request",
				ntohl((((packet_game_log_request*)data)->gameid)));
			send_game_log(current_session->client_socket, ntohl(((packet_game_log_request*)data)->gameid));
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
	new_session->player = NULL;
	
	pthread_mutex_lock(&current_lobby.sessions->locking_mutex);
	dynamic_array_add(current_lobby.sessions, new_session);
	pthread_mutex_unlock(&current_lobby.sessions->locking_mutex);
	
	if( (pthread_create(&new_session->thread, NULL, (void*)Session, (void*)new_session)) != 0){
		perror("pthread_create");
		exit(1);
	}
}
