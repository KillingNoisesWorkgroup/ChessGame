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
