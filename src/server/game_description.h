#ifndef H_SERVER_GAME_DESCRIPTION_GUARD
#define H_SERVER_GAME_DESCRIPTION_GUARD

#define GAME_STATE_INITIAL_GAME 0

typedef struct game_description{
	int state;
	int id;
	int player1_id;
	int player2_id;
} game_description;

#endif

