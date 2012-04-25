#ifndef H_SERVER_NETWORKING_GUARD
#define H_SERVER_NETWORKING_GUARD

#include <stdint.h>

// Turn on packing mode of the compiler
#pragma pack(push, 1)

// Including packets with packing...
#include "packets.h"

// Turn off packing mode of the compiler
#pragma pack(pop)


typedef uint8_t packet_type_t;
typedef uint16_t packet_length_t;

// Network functions wrappers
void packet_send(int dst, packet_type_t packet_type, packet_length_t length, void *raw_data);
int packet_recv(int src, packet_type_t *packet_type, packet_length_t *length, void **data);

#endif

