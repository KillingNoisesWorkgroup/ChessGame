#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>

#include "networking.h"

void packet_send(int dst, packet_type_t packet_type, packet_length_t length, void *raw_data){
	char *data;
	char *p; // just a helper pointer
	
	if((data = malloc(sizeof(packet_type) + sizeof(length) + length)) == NULL){
		perror("malloc");
		exit(1);
	}
	p = data;
	
	*(packet_type_t*)p = packet_type;
	p += sizeof(packet_type);
	
	*(packet_length_t*)p = length;
	p += sizeof(length);
	
	memcpy(p, raw_data, length);
	
	if((send(dst, data, sizeof(packet_type) + sizeof(length) + length, 0) == -1)){
		perror("send");
		exit(1);
	}
	
	free(data);
}

int packet_recv(int src, packet_type_t *packet_type, packet_length_t *length, void **data){
	if((recv(src, packet_type, sizeof(packet_type_t), MSG_WAITALL)) == -1) {
		if(errno == ECONNREFUSED) {
			return 0;
		} else {
			perror("recv");
			exit(1);
		}
	}
	
	if((recv(src, length, sizeof(packet_length_t), MSG_WAITALL)) == -1) {
		if(errno == ECONNREFUSED) {
			return 0;
		} else {
			perror("recv");
			exit(1);
		}
	}
	
	if((*data = malloc(*length)) == NULL){
		perror("malloc");
		exit(1);
	}
	
	if((recv(src, *data, *length, MSG_WAITALL)) == -1) {
		if(errno == ECONNREFUSED) {
			return 0;
		} else {
			perror("recv");
			exit(1);
		}
	}	
	return 1;
}

