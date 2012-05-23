// All data in this file is packed for lowest space
// Do NOT include this file manually! Include networking.h instead!

#define MAX_PACKET_STRING_LENGTH 256
#define MAX_CHAT_MESSAGE_LENGTH 128 + 1

/* Dynamic length packet types */

#define PACKET_GENERAL_STRING 100


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
	uint32_t userid;
} packet_auth_response;


/* PACKET C2S - Server shutdown request (admin only) */

#define PACKET_SERVER_SHUTDOWN 3
typedef struct packet_server_shutdown{
	// empty
} packet_server_shutdown;


/* PACKET C2S - Game creation request */

#define PACKET_GAME_CREATION_REQUEST 4
typedef struct packet_game_creation_request{
	char name[MAX_PACKET_STRING_LENGTH];
} packet_game_creation_request;


/* PACKET S2C - Game creation response */

#define PACKET_GAME_CREATION_RESPONSE 5
typedef struct packet_game_creation_response{
	uint32_t gameid;
} packet_game_creation_response;


/* PACKET C2S - Game attach request */

#define TEAM_AUTO        0
#define TEAM_WHITE       1
#define TEAM_BLACK       2
#define TEAM_SPECTATORS  3

#define PACKET_GAME_ATTACH_REQUEST 6
typedef struct packet_game_attach_request{
	uint32_t gameid;
	uint8_t team;
} packet_game_attach_request;


/* PACKET S2C - Game attach response */

#define PACKET_GAME_ATTACH 7
typedef struct packet_game_attach{
	uint32_t gameid;
	uint8_t attached_as_team;
	char game_name[MAX_PACKET_STRING_LENGTH];
} packet_game_attach;


/* PACKET C2S - Games list request */

#define PACKET_GAMES_LIST_REQUEST 8
typedef struct packet_games_list_request{
	// empty
} packet_games_list_request;


/* PACKET C2S - Game detach request */

#define PACKET_GAME_DETACH_REQUEST 9
typedef struct packet_game_detach_request{
	// empty
} packet_game_detach_request;


/* PACKET S2C - Game detach response */

#define PACKET_GAME_DETACH 10
typedef struct packet_game_detach{
	// empty
} packet_game_detach;


/* PACKET S2C - Game desk */

#define PACKET_GAME_DESK 11
typedef struct packet_game_desk{
	desk_t desk;
} packet_game_desk;


/* PACKET C2S - Figure move */

#define PACKET_FIGURE_MOVE 12
typedef struct packet_figure_move{
	uint8_t from_number;
	uint8_t from_letter;
	uint8_t to_number;
	uint8_t to_letter;	
} packet_figure_move;


/* PACKET C2S - Game delete request (admin only) */

#define PACKET_GAME_DELETE_REQUEST 13
typedef struct packet_game_delete_request{
	uint32_t gameid;
} packet_game_delete_request;


/* PACKET C2S - Users list request */

#define PACKET_USERS_LIST_REQUEST 14
typedef struct packet_users_list_request{
	uint8_t online_only;
} packet_users_list_request;


/* PACKET C2S - Games history request */

#define PACKET_GAMES_HISTORY_REQUEST 15
typedef struct packet_games_history_request{
	// empty
} packet_games_history_request;


/* PACKET C2S - Game log request */

#define PACKET_GAME_LOG_REQUEST 16
typedef struct packet_game_log_request{
	uint32_t gameid;
} packet_game_log_request;


/* PACKET C2S - User rating request */

#define PACKET_USER_RATING_REQUEST 17
typedef struct packet_user_rating_request{
	uint32_t userid;
} packet_user_rating_request;


/* PACKET C2S - Kick from game request (admin only) */

#define PACKET_KICK_FROM_GAME_REQUEST 18
typedef struct packet_kick_from_game_request{
	uint32_t gameid;
	uint32_t userid;
} packet_kick_from_game_request;


/* PACKET C2S - Matchmaking queue request */

#define PACKET_MATCHMAKING_QUEUE_REQUEST 19
typedef struct packet_matchmaking_queue_request{
	// empty
} packet_matchmaking_queue_request;


/* PACKET C2S - Chat message outgoing */

#define PACKET_CHAT_MESSAGE_OUTGOING 20
typedef struct packet_chat_message_outgoing{
	char text[MAX_CHAT_MESSAGE_LENGTH];
} packet_chat_message_outgoing;


/* PACKET C2S - User delete request (admin only) */

#define PACKET_USER_DELETE_REQUEST 21
typedef struct packet_user_delete_request{
	uint32_t userid;
} packet_user_delete_request;
