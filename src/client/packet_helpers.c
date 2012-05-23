#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "helpers.h"
#include "packet_helpers.h"

#include "../shared/networking.h"

int send_shutdown(int dst) {
	return packet_send(dst, PACKET_SERVER_SHUTDOWN, 0, NULL);
}

int send_game_creation_request(int dst, char *name) {
	packet_game_creation_request packet;
	strncpy(packet.name, name, sizeof packet.name);
	return packet_send(dst, PACKET_GAME_CREATION_REQUEST, sizeof packet, &packet);
}

int send_game_attach_request(int dst, uint32_t gameid, uint8_t team) {
	packet_game_attach_request packet;
	packet.team = team;
	packet.gameid = htonl(gameid);
	return packet_send(dst, PACKET_GAME_ATTACH_REQUEST, sizeof packet, &packet);
}

int send_games_list_request(int dst) {
	return packet_send(dst, PACKET_GAMES_LIST_REQUEST, 0, NULL);
}

int send_game_detach_request(int dst) {
	return packet_send(dst, PACKET_GAME_DETACH_REQUEST, 0, NULL);
}

int send_figure_move(int dst, uint8_t from_letter, uint8_t from_number, uint8_t to_letter, uint8_t to_number) {
	packet_figure_move packet;
	packet.from_letter = from_letter;
	packet.from_number = from_number;
	packet.to_letter = to_letter;
	packet.to_number = to_number;
	return packet_send(dst, PACKET_FIGURE_MOVE, sizeof packet, &packet);
}

int send_games_history_request(int dst) {
	return packet_send(dst, PACKET_GAMES_HISTORY_REQUEST, 0, NULL);
}

int send_matchmaking_queue_request(int dst) {
	return packet_send(dst, PACKET_MATCHMAKING_QUEUE_REQUEST, 0, NULL);
}

int send_chat_message_outgoing(int dst, char *text) {
	packet_chat_message_outgoing packet;
	strncpy(packet.text, text, sizeof packet.text);
	return packet_send(dst, PACKET_CHAT_MESSAGE_OUTGOING, sizeof packet, &packet);
}

int send_game_delete_request(int dst, uint32_t gameid) {
	packet_game_delete_request packet;
	packet.gameid = htonl(gameid);
	return packet_send(dst, PACKET_GAME_DELETE_REQUEST, sizeof packet, &packet);
}

int send_users_list_request(int dst, uint8_t online_only) {
	packet_users_list_request packet;
	packet.online_only = online_only;
	return packet_send(dst, PACKET_USERS_LIST_REQUEST, sizeof packet, &packet);
}

int send_game_log_request(int dst, uint32_t gameid) {
	packet_game_log_request packet;
	packet.gameid = htonl(gameid);
	return packet_send(dst, PACKET_GAME_LOG_REQUEST, sizeof packet, &packet);
}

int send_user_rating_request(int dst, uint32_t userid) {
	packet_user_rating_request packet;
	packet.userid = htonl(userid);
	return packet_send(dst, PACKET_USER_RATING_REQUEST, sizeof packet, &packet);
}

int send_kick_from_game_request(int dst, uint32_t gameid, uint32_t userid) {
	packet_kick_from_game_request packet;
	packet.gameid = htonl(gameid);
	packet.userid = htonl(userid);
	return packet_send(dst, PACKET_KICK_FROM_GAME_REQUEST, sizeof packet, &packet);
}
