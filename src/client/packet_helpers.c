#include <stdlib.h>

#include "helpers.h"
#include "packet_helpers.h"

#include "../shared/networking.h"

int send_shutdown(int dst) {
	return packet_send(dst, PACKET_SERVER_SHUTDOWN, 0, NULL);
}
