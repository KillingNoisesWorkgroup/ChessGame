#ifndef H_SERVER_GAME_LOG_GUARD
#define H_SERVER_GAME_LOG_GUARD

#define GAME_LOG_DIR "./game_logs/"

#include <stdio.h>
#include <stdlib.h>

#include "game_description.h"
#include "../shared/networking.h"

FILE * open_game_log(uint32_t id);
void game_log_move(game_description * game, packet_figure_move *move);

#endif
