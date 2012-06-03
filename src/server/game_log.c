#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "game_log.h"
#include "../shared/helpers.h"

FILE* open_game_log(uint32_t id){
	FILE* gl;
	char gl_name[256], tmp[11];
	
	sprintf(tmp, "%010d", id);
	tmp[11] = '\0';
	gl_name[0] = '\0';
	strcat(gl_name, GAME_LOG_DIR);
	strcat(gl_name, tmp);
	
	mkdir(GAME_LOG_DIR);
	
	if((gl = fopen(gl_name, "a+")) == NULL){
		perror("fopen");
		exit(1);
	}
	
	return gl;
}

void game_log_move(game_description *game, packet_figure_move *move){
	if(game->moves_made % 2 == 0){
		fprintf(game->game_log, "%3d. %c%d:%c%d ", game->moves_made/2,
			num_to_char((int)(move->from_letter+1)), move->from_number+1,
			num_to_char((int)(move->to_letter+1)), move->to_number+1);
		fflush(game->game_log);
	} else {
		fprintf(game->game_log, "%c%d:%c%d\n", 
			num_to_char((int)(move->from_letter+1)), move->from_number+1,
			num_to_char((int)(move->to_letter+1)), move->to_number+1);
		fflush(game->game_log);
	}
}
