#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "../shared/networking.h"
#include "login_entry.h"
#include "session.h"
#include "lobby.h"
#include "game_description.h"
#include "dynamic_array.h"

#define CONNECTION_REQUEST_QUEUE_BACKLOG_SIZE 10

#define PORT 30205

int server_socket;
struct sockaddr_in server_addres;

int main(int argc, char **args){
	int client_socket;
	struct sockaddr client_addres;
	socklen_t client_socklen;
	
	if( (server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("socket");
		exit(1);
	}
	
	server_addres.sin_family = PF_INET;
	server_addres.sin_port = htons(PORT);
	server_addres.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( bind(server_socket, (struct sockaddr *)(&server_addres), sizeof(struct sockaddr_in)) == -1 ){
		perror("bind");
		exit(1);
	}
	
	create_lobby();
	read_passwords();
	//print_passwords();
	
	listen(server_socket, CONNECTION_REQUEST_QUEUE_BACKLOG_SIZE);
	
	printf("Listening on 0.0.0.0:%d\n", PORT);
	
	while(1){
		client_socket = accept(server_socket, &client_addres, &client_socklen);
		create_session(client_socket, &client_addres);
	}
	return 0;
}
