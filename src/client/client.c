#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <openssl/md5.h>

#include "client.h"
#include "helpers.h"
#include "callbacks.h"

#include "../shared/shared.h"
#include "../shared/networking.h"

/*
void user_communications(void) {
	char *buff;
	int tmp[4];
	
	//fclose(stdout);
	while(1) {		
		fflush(stdout);	
		// printf("-> ");
		
		// Read and send commands
		scanf("%as", &buff);		
		fflush(stdout);	
		
		if(strcmp(buff, "imove") == 0) {
		
			scanf("%d", &tmp[0]);
			scanf("%d", &tmp[1]);
			scanf("%d", &tmp[2]);
			scanf("%d", &tmp[3]);
			
			printf("\nMoving [%d,%d] to [%d,%d]\n", tmp[0], tmp[1], tmp[2], tmp[3]);
			//move_figure(tmp[0], tmp[1], tmp[2], tmp[3]);
			printf("-> ");
			
		} else if(strcmp(buff, "move") == 0) {
		
			scanf("%as", &buff);
			if(strlen(buff) != 2) {
				printf("Wrong argument!\n");
				continue;
			}
			
			tmp[1] = char_to_num(buff[0]) - 1;
			tmp[0] = atoi(&buff[1]) - 1;
			
			scanf("%as", &buff);
			if(strlen(buff) != 2) {
				printf("Wrong argument!\n");
				continue;
			}
			
			tmp[3] = char_to_num(buff[0]) - 1;
			tmp[2] = atoi(&buff[1]) - 1;
						
			printf("\nMoving [%d,%d] to [%d,%d]\n", tmp[0], tmp[1], tmp[2], tmp[3]);
			//move_figure(tmp[0], tmp[1], tmp[2], tmp[3]);
			printf("-> ");
			
		} else if(strcmp(buff, "redraw") == 0) {
					
			//print_desk(fp_shmem->cells);
			
		} else if(strcmp(buff, "exit") == 0) {
		
			//send_simple(FP_OPERATION_DISCONNECTED);
			printf("Bye!\n");
			break;
			
		} else if(strcmp(buff, "shutdown") == 0) {
			
			//send_simple(FP_OPERATION_EXIT);
			printf("Sent server shutdown!\n");			
			printf("Bye!\n");
			break;	
			
		} else if(strcmp(buff, "restart") == 0) {
			
			//send_simple(FP_OPERATION_RESTART);
			printf("Sent server restart!\n");			
			printf("Bye!\n");
			break;	
			
		} else {
			
			printf("Unknown command!\n");
			printf("-> ");
		
		}
		
		fflush(stdout);
	}
}
*/

void connect_to_server() {
	struct sockaddr_in address;

	if((session.socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	bzero((char *)&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(session.server.port);
    bcopy((char *)session.server.host->h_addr, (char *)&address.sin_addr.s_addr, session.server.host->h_length);
	
	if(connect(session.socket, (struct sockaddr *) &address, sizeof(address))) {
		perror("connect");
		exit(EXIT_FAILURE);
	}
}

// Blocks the whole thing untill auth
void authenticate() {
	// Send auth request
	packet_auth_request packet;
	strncpy(packet.login, session.login, PLAYER_NAME_MAXSIZE);
	memcpy(packet.passw, session.password_encrypted, ENCRYPTED_PASSWORD_LENGTH);
	
	packet_send(session.socket, PACKET_AUTH_REQUEST, sizeof(packet), &packet);
	
	
	// Handle auth response
	packet_type_t ptype;
	packet_length_t plen;
	void *payload;
	
	if(!packet_recv(session.socket, &ptype, &plen, &payload) || ptype != PACKET_AUTH_RESPONSE || !((packet_auth_response*)payload)->response) {
		fprintf(stderr, "Unable to authenticate!\n");
		exit(EXIT_FAILURE);
	}
}


void input_thread_remote(void *arg) {
	packet_type_t ptype;
	packet_length_t plen;
	void *payload;
	
	while(1) {
		// Read from socket with blocking
		if(!packet_recv(session.socket, &ptype, &plen, &payload)) { // Disconnected
			fprintf(stderr, "Disconnected from server!\n");
			exit(3);
		}		
		
		pthread_mutex_lock(&reactor.locking_mutex);
		
		// Do all the stuff in here
		printf("Remote input got packet!\n");
		packet_debug(ptype, plen, payload);
		
		pthread_mutex_unlock(&reactor.locking_mutex);

		// No leaks here! :)
		free(payload);
	}
}

void input_thread_local(void *arg) {
	char *buff;
	int len;
	int read;
	
	while(1) {
		// Read from input stream with blocking
		print_prompt();	
		if(!get_command(&buff, &len)) {
			printf("EOF\n");
			exit(0);
		}
		
		pthread_mutex_lock(&reactor.locking_mutex);
		
		// Do all the stuff in here
		printf("Command: %s\n", buff);
		
		pthread_mutex_unlock(&reactor.locking_mutex);
		
		free(buff);
	}
}

// Should terminate all threads
void terminate() {
	// This will terminate all threads for sure...
	exit(EXIT_SUCCESS);
}

int main(int args, char **argv) {
	if(args != 5) {
		printf("Usage: %s host port login password\n", argv[0]);
		printf("Connects to chess server.\n");
		exit(EXIT_SUCCESS);
	}

	if((session.server.host = gethostbyname(argv[1])) == NULL) {
		printf("Wrong host %s!\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	session.server.port = atoi(argv[2]);
	
	strncpy((char*)&session.login, argv[3], PLAYER_NAME_MAXSIZE-1);
	session.login[PLAYER_NAME_MAXSIZE-1] = '\0';
	
	MD5((unsigned char *) argv[4], strlen(argv[4]), (unsigned char*)&session.password_encrypted);
	
	printf("Connecting...\n");
	
	connect_to_server();
	
	printf("Authenticating...\n");
	
	authenticate();
	
	printf("Success!\n");
	
	if(pthread_create(&reactor.thread_input_local, NULL, (void*)input_thread_local, NULL) ||
		pthread_create(&reactor.thread_input_remote, NULL, (void*)input_thread_remote, NULL)) {
		fprintf(stderr, "Unable to create thread!\n");
		exit(EXIT_FAILURE);
	}
	
	// Working...
	
	pthread_join(reactor.thread_input_local, NULL);
	pthread_join(reactor.thread_input_remote, NULL);
	
	return EXIT_SUCCESS;
}

