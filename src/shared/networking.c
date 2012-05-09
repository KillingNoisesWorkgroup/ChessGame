#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#include "networking.h"

int packet_send(int dst, packet_type_t packet_type, packet_length_t length, void *raw_data){
	char *data;
	char *p; // just a helper pointer
	
	if((data = malloc(sizeof(packet_type) + sizeof(length) + length)) == NULL){
		perror("malloc");
		exit(1);
	}
	p = data;
	
	*(packet_type_t*)p = packet_type;
	p += sizeof(packet_type);
	
	*(packet_length_t*)p = htons(length);
	p += sizeof(length);
	
	if(length > 0 && raw_data)
		memcpy(p, raw_data, length);
	
	if((send(dst, data, sizeof(packet_type) + sizeof(length) + length, 0) == -1)){
		perror("send");
		exit(1);
	}
	
	free(data);
	return 1;
}

int packet_recv(int src, packet_type_t *packet_type, packet_length_t *length, void **data){
	fd_set rfds;
	int retval;
	
	*data = NULL;
	
	FD_ZERO(&rfds);
	FD_SET(src, &rfds);
	if ((retval = select(src + 1, &rfds, NULL, NULL, NULL)) == -1) {
		perror("select");
		exit(1);
	}

	if((retval = recv(src, packet_type, sizeof(packet_type_t), MSG_WAITALL)) == -1) {
		if(errno == ECONNREFUSED) {
			return 0;
		} else {
			perror("recv1");
			exit(1);
		}
	}
	if(retval == 0) return 0;
	
	FD_ZERO(&rfds);
	FD_SET(src, &rfds);
	if ((retval = select(src + 1, &rfds, NULL, NULL, NULL)) == -1) {
		perror("select");
		exit(1);
	}
	
	if((retval = recv(src, length, sizeof(packet_length_t), MSG_WAITALL)) == -1) {
		if(errno == ECONNREFUSED) {
			return 0;
		} else {
			perror("recv2");
			exit(1);
		}
	}
	if(retval == 0) return 0;
	
	*length = ntohs(*length);
	
	if(*length > 0) {
		FD_ZERO(&rfds);
		FD_SET(src, &rfds);
		if ((retval = select(src + 1, &rfds, NULL, NULL, NULL)) == -1) {
			perror("select");
			exit(1);
		}
		
		if((*data = malloc(*length)) == NULL){
			perror("malloc");
			exit(1);
		}
		
		if((retval = recv(src, *data, *length, MSG_WAITALL)) == -1) {
			if(errno == ECONNREFUSED) {
				free(*data);
				return 0;
			} else {
				perror("recv3");
				exit(1);
			}
		}
		
		if(retval == 0) {
			free(*data);
			return 0;
		}
	}
	
	return 1;
}

void packet_debug(packet_type_t packet_type, packet_length_t packet_length, void *data) {
	int i;
	printf("Packet (%d bytes, type %d): \n", packet_length, packet_type);
	for(i = 0; i < packet_length; i++) {
		if(i == 64) {
			printf("...");
			break;
		}
		printf("%02X ", ((unsigned char*)data)[i]);
		if((i + 1) % 8 == 0) printf("\n");
	}
	printf("\n---------------\n");
}

void packet_debug_full(packet_type_t packet_type, packet_length_t packet_length, void *data) {
	int i;
	printf("Packet (%d bytes, type %d): \n", packet_length, packet_type);
	for(i = 0; i < packet_length; i++) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if((i + 1) % 8 == 0) printf("\n");
	}
	printf("\n---------------\n");
}
