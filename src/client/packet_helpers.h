#ifndef H_CLIENT_PACKET_HELPERS_GUARD
#define H_CLIENT_PACKET_HELPERS_GUARD

int send_shutdown(int dst);
int send_game_creation_request(int dst, char *name);
int send_game_attach_request(int dst, uint32_t gameid, uint8_t team);

#endif
