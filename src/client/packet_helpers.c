#include <stdlib.h>

#include "helpers.h"
#include "packet_helpers.h"

#include "../shared/networking.h"

int send_shutdown(int dst) {
	packet_server_shutdown packet;
	packet_send(dst, PACKET_SERVER_SHUTDOWN, sizeof packet, &packet);
	return 1;
}