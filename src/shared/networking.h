#ifndef H_SHARED_NETWORKING_GUARD
#define H_SHARED_NETWORKING_GUARD

#include <stdint.h>
#include <openssl/md5.h>

// Turn on packing mode of the compiler
#pragma pack(push, 1)

// Including packets with packing...
#include "packets.h"

// Turn off packing mode of the compiler
#pragma pack(pop)


typedef uint8_t packet_type_t;
typedef uint16_t packet_length_t;

// Network functions wrappers
int packet_send(int dst, packet_type_t packet_type, packet_length_t length, void *raw_data);
int packet_recv(int src, packet_type_t *packet_type, packet_length_t *length, void **data);
void packet_debug(packet_type_t packet_type, packet_length_t packet_length, void *data);
void packet_debug_full(packet_type_t packet_type, packet_length_t packet_length, void *data);

#endif

