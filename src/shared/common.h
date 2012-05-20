#ifndef H_SHARED_COMMON_GUARD
#define H_SHARED_COMMON_GUARD

#include <stdint.h>

// content must be allocated and freed manually
// length must be converted to network byte-order if transferring over
typedef struct binary_data_t {
	uint32_t length;
	char * content;
} binary_data_t;

typedef struct deck{
	cell cells[64];
} deck;

typedef struct figure{
	uint8_t type;
	uint8_t team;
} figure;

typedef struct cell {
	uint8_t isempty;
	figure f;
} cell;

#endif
