#include <stdlib.h>
#include <string.h>

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
