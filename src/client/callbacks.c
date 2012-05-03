#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "helpers.h"
#include "callbacks.h"

// Some helpers to fancy up the code
#define on if(0){}
#define command(cmd) else if(check_command(buff, (cmd)))
#define packet(type) else if(ptype == type)

// Variable names must be all the same as in the prototype for 
// fancy helpers to work properly

// Default callbacks

void cb_remote_default(int ptype, int plen, void *payload) {
	on else {
		printf("Unknown packet!\n");
		packet_debug(ptype, plen, payload);
	}
}

void cb_local_default(char *buff, int len) {	
	on command("help") {
		
		printf("Commands list:\n");
		printf("  help\n");
		printf("  shutdown\n");
		printf("  exit\n");
	
	} command("shutdown") {
	
		printf("Shutting the server down...\n");
		// TODO: send shutdown packet
		
	} command("exit") {
	
		printf("Bye!\n");
		exit(EXIT_SUCCESS);
		
	} else {
		printf("Unknown command!\n");
	}
}
