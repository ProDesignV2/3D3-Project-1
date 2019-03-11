#ifndef HELPER_H
#define HELPER_H

// Helper functions for main program

#include <sys/socket.h>

/*
#define MAX_ADDR_LENGTH 200
#define MAX_PORT_LENGTH 5
#define MAX_FILE_LENGTH 200
*/

struct URL_Parsed
{	
	/*
	char addr[MAX_ADDR_LENGTH];
	char port[MAX_PORT_LENGTH];
	char file[MAX_FILE_LENGTH];
	*/
	char *addr;
	char *port;
	char *file;
};

void *
get_in_addr(struct sockaddr *sa);

unsigned short int
get_in_port(struct sockaddr *sa);

int
send_all(int send_fd, const char *buf, int *len);

URL_Parsed
parse_url(char *url);

#endif
