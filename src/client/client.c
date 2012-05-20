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

#include "../shared/networking.h"

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
	
	if(!packet_recv(session.socket, &ptype, &plen, &payload) || ptype != PACKET_AUTH_RESPONSE || ntohl(((packet_auth_response*)payload)->userid) <= 0) {
		fprintf(stderr, "Unable to authenticate!\n");
		exit(EXIT_FAILURE);
	}
	
	session.userid = ntohl(((packet_auth_response*)payload)->userid);
}

void common_init() {
	session.state.current = GAMESTATE_LOBBY;
}

void reactor_init() {
	reactor.callback_remote = NULL;
	reactor.callback_local = NULL;
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
		//printf("Remote input got packet!\n");
		//packet_debug(ptype, plen, payload);
		
		if(reactor.callback_remote) {
			reactor.callback_remote(ptype, plen, payload);
		} else {
			cb_remote_default(ptype, plen, payload);
		}
		
		pthread_mutex_unlock(&reactor.locking_mutex);

		// No leaks here! :)
		free(payload);
	}
}

void input_thread_local(void *arg) {
	char *buff;
	size_t len;
	
	while(1) {
		// Read from input stream with blocking
		if(ommit_next_autoprompt)
			ommit_next_autoprompt--;
		else
			print_prompt();
			

		if(!get_string(&buff, &len)) {
			fprintf(stderr, "\nEOF\n");
			exit(EXIT_SUCCESS);
		}
		
		pthread_mutex_lock(&reactor.locking_mutex);
		
		// Do all the stuff in here
		//printf("Command: %s", buff);
		
		if(reactor.callback_local) {
			reactor.callback_local(buff, len);
		} else {
			cb_local_default(buff, len);
		}
		
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
	
	output(L"Connecting...\n");
	
	connect_to_server();
	
	output(L"Authenticating...\n");
	
	authenticate();
	
	output(L"Success!\n");
	
	reactor_init();
	common_init();
	
	// Create threads
	
	pthread_mutex_init(&reactor.locking_mutex, NULL);
	
	if(pthread_create(&reactor.thread_input_local, NULL, (void*)input_thread_local, NULL) ||
		pthread_create(&reactor.thread_input_remote, NULL, (void*)input_thread_remote, NULL)) {
		fprintf(stderr, "Unable to create thread!\n");
		exit(EXIT_FAILURE);
	}
	
	// Working...
	
	
	// Join threads
	
	pthread_join(reactor.thread_input_local, NULL);
	pthread_join(reactor.thread_input_remote, NULL);
	
	pthread_mutex_destroy(&reactor.locking_mutex);
	
	return EXIT_SUCCESS;
}

