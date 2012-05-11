#ifndef H_SHARED_COMMON_GUARD
#define H_SHARED_COMMON_GUARD

#include <stdint.h>

// content must be allocated and freed manually
// length must be converted to network byte-order if transferring over
typedef struct binary_data_t {
	uint32_t length;
	char * content;
} binary_data_t;

#endif

