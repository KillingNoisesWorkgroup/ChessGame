// All data in this file is packed for lowest space
// Do NOT include this file manually! Include networking.h instead!

/* PACKET C2S - Auth Request */

#define PLAYER_NAME_MAXSIZE 256
#define ENCRYPTED_PASSWORD_LENGTH 32

#define PACKET_AUTH_REQUEST 1
typedef struct packet_auth_request{
	char login[PLAYER_NAME_MAXSIZE];
	char passw[ENCRYPTED_PASSWORD_LENGTH];
} packet_auth_request;
