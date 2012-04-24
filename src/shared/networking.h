#ifndef H_SERVER_NETWORKING_GUARD
#define H_SERVER_NETWORKING_GUARD

/* PACKET C2S - Auth Request */

#define PLAYER_NAME_MAXSIZE 256
#define ENCRYPTED_PASSWORD_LENGTH 32

#define PACKET_AUTH_REQUEST 1
typedef struct packet_auth_request{
	char login[PLAYER_NAME_MAXSIZE];
	char passw[ENCRYPTED_PASSWORD_LENGTH];
} packet_auth_request;

void packet_send(int dst, int packet_type, int length, void *raw_data);
void packet_recv(int src, int *packet_type, int *length, void **data);

#endif

