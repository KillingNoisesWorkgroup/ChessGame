// All data in this file is packed for lowest space
// Do NOT include this file manually! Include networking.h instead!

/* PACKET C2S - Auth Request */

#define PLAYER_NAME_MAXSIZE 256
#define ENCRYPTED_PASSWORD_LENGTH MD5_DIGEST_LENGTH

#define PACKET_AUTH_REQUEST 1
typedef struct packet_auth_request{
	char login[PLAYER_NAME_MAXSIZE];
	char passw[ENCRYPTED_PASSWORD_LENGTH];
} packet_auth_request;


/* PACKET S2C - Auth Response */

#define PACKET_AUTH_RESPONSE 2
typedef struct packet_auth_response{
	uint8_t response;
} packet_auth_response;


/* PACKET C2S - Server shutdown request (admin only) */

#define PACKET_SERVER_SHUTDOWN 3
typedef struct packet_server_shutdown{
	// empty
} packet_server_shutdown;


/* PACKET C2S - Game creation request */

#define PACKET_GAME_CREATION_REQUEST 4
typedef struct packet_game_creation_request{
	char name;
	uint8_t spect_num;
} packet_game_creation_request;


/* PACKET S2C - Game creation response */

#define PACKET_GAME_CREATION_RESPONSE 5
typedef struct packet_game_creation_response{
	uint8_t response;
} packet_game_creation_response;
