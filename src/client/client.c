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

#include "../shared/shared.h"
#include "../shared/networking.h"

typedef struct server_rec {
	struct hostent *host;
	int port;
} server_rec;

typedef struct session_rec {
	server_rec server;

	char login[PLAYER_NAME_MAXSIZE];
	unsigned char password_encrypted[ENCRYPTED_PASSWORD_LENGTH];
	
	int socket;
} session_rec;

// This is NOT a real reactor.
typedef struct reactor_rec {
	pthread_t thread_input_remote;
	pthread_t thread_input_local;
	pthread_mutex_t locking_mutex;
} reactor_rec;

session_rec session;
reactor_rec reactor;

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

	if((session.socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	bzero((char *) &address, sizeof(address));
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
	packet_auth_request packet;
	strncpy(packet.login, session.login, PLAYER_NAME_MAXSIZE);
	strncpy(packet.passw, session.password_encrypted, ENCRYPTED_PASSWORD_LENGTH);
	
	packet_send(session.socket, PACKET_AUTH_REQUEST, sizeof(packet), &packet);
	// Lol, is that all?
}


void input_thread_remote(void *arg) {
	packet_type_t packet_type;
	packet_length_t length;
	void *data;
	
	while(1) {		
		// Read from socket with blocking
		if(!packet_recv(session.socket, &packet_type, &length, &data)) { // Disconnected
			fprintf(stderr, "Disconnected from server!\n");
			exit(3);
		}		
		
		pthread_mutex_lock(&reactor.locking_mutex);
		
		// Do all the stuff in here
		printf("Remote input got packet!\n");
		
		pthread_mutex_unlock(&reactor.locking_mutex);

		// No leaks here! :)
		free(data);
	}
}

void input_thread_local(void *arg) {
	char *input;
	while(1) {
		// Read from input stream with blocking
		scanf("%as", &input);		
		
		pthread_mutex_lock(&reactor.locking_mutex);
		
		// Do all the stuff in here
		
		pthread_mutex_unlock(&reactor.locking_mutex);
	}
}

// Should terminate all threads
void terminate() {
	// This will terminate all threads for sure...
	exit(EXIT_SUCCESS);
}

int main(int args, unsigned char **argv) {
	int forkpid;

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
	
	MD5(argv[4], strlen(argv[4]), (unsigned char*)&session.password_encrypted);
	
	printf("Connecting...\n");
	
	connect_to_server();
	
	printf("Connected!\n");
	
	authenticate();
	
	printf("Success!\n");
	
	if(pthread_create(&reactor.thread_input_local, NULL, (void*)input_thread_local, NULL) ||
		pthread_create(&reactor.thread_input_remote, NULL, (void*)input_thread_remote, NULL)) {
		fprintf(stderr, "Unable to create thread!\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Working!\n");
	
	pthread_join(reactor.thread_input_local, NULL);
	pthread_join(reactor.thread_input_remote, NULL);
	
	return EXIT_SUCCESS;
}

