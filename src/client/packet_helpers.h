#ifndef H_CLIENT_PACKET_HELPERS_GUARD
#define H_CLIENT_PACKET_HELPERS_GUARD

int send_shutdown(int dst);
int send_game_creation_request(int dst, char *name);

#endif
