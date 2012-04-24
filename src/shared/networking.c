#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "networking.h"

void packet_send(int dst, int packet_type, int length, void *raw_data){
	char* data;
	if( (data = malloc(length + 2*sizeof(int))) == NULL ){
		perror("malloc");
		exit(1);
	}
	((int*)data)[0] = packet_type;
	((int*)data)[1] = length;
	memcpy(&(((int*)data)[2]), raw_data, length);
	if( (send(dst, data, length + 2*sizeof(int), 0) == -1 ) ){
		perror("send");
		exit(1);
	}
	free(data);
}

void packet_recv(int src, int *packet_type, int *length, void **data){
	if( (length = malloc(sizeof(int))) == NULL ){
		perror("malloc");
		exit(1);
	}
	if( (packet_type = malloc(sizeof(int))) == NULL ){
		perror("malloc");
		exit(1);
	}
	if( (recv(src, packet_type, sizeof(int), 0)) == -1 ) {
		perror("recv");
		exit(1);
	}
	if( (recv(src, length, sizeof(int), 0)) == -1 ) {
		perror("recv");
		exit(1);
	}
	if( (*data = malloc(*length)) == NULL){
		perror("malloc");
		exit(1);
	}
	if( (recv(src, *data, *length, 0)) == -1 ) {
		perror("recv");
		exit(1);
	}
}

