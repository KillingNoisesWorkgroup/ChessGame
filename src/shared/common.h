#ifndef H_SHARED_COMMON_GUARD
#define H_SHARED_COMMON_GUARD

#include <stdint.h>


/* Common data */

// content must be allocated and freed manually
// length must be converted to network byte-order if transferring over
typedef struct binary_data_t {
	uint32_t length;
	char * content;
} binary_data_t;


/* Game data */

// Figures
#define FIGURE_NONE   0
#define FIGURE_KING   1
#define FIGURE_QUEEN  2
#define FIGURE_ROOK   3
#define FIGURE_KNIGHT 4
#define FIGURE_BISHOP 5
#define FIGURE_PAWNS  6

// Game desk representation, ready to transfer over the network
#pragma pack(push, 1)
typedef struct figure {
	uint8_t type;
	uint8_t team;
} figure;

typedef figure cell;

typedef struct desk {
	cell cells[64];
} desk;
#pragma pack(pop)


#endif
