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
#define FIGURE_PAWN   6

#define FIGURE_COLOR_WHITE 0
#define FIGURE_COLOR_BLACK 1

// Game desk representation, ready to transfer over the network
#pragma pack(push, 1)
typedef struct cell_t {
	uint8_t type;
	uint8_t color;
} cell_t;

/*
  Desk should be represented like this:
      A  B  C  D  E  F  G  H
   1  0  1  2  3  4  5  6  7
   2  8  9 10 11 12 13 14 15
   3 16 ...
   4 24 ...
   5 32 ...
   6 40 ...
   7 48 ...
   8 56 57 58 59 60 61 62 63
   
*/

typedef struct desk_t {
	cell_t cells[64];
} desk_t;
#pragma pack(pop)


#endif
