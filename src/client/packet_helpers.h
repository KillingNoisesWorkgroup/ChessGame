#ifndef H_CLIENT_PACKET_HELPERS_GUARD
#define H_CLIENT_PACKET_HELPERS_GUARD

int send_shutdown(int dst);
int send_game_creation_request(int dst, char *name);
int send_game_attach_request(int dst, uint32_t gameid, uint8_t team);
int send_games_list_request(int dst);
int send_game_detach_request(int dst);
int send_figure_move(int dst, uint8_t from_letter, uint8_t from_number, uint8_t to_letter, uint8_t to_number);
int send_games_history_request(int dst);
int send_matchmaking_queue_request(int dst);
int send_chat_message_outgoing(int dst, char *text);
int send_game_delete_request(int dst, uint32_t gameid);
int send_users_list_request(int dst, uint8_t online_only);
int send_game_log_request(int dst, uint32_t gameid);
int send_user_rating_request(int dst, uint32_t userid);
int send_kick_from_game_request(int dst, uint32_t gameid, uint32_t userid);
int send_user_delete_request(int dst, uint32_t userid);

#endif
