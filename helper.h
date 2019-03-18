#ifndef HELPER_H
#define HELPER_H

// Helper functions for main program

#include <sys/socket.h>

enum class Persistent
{
	pers,
	non_pers,
	pers_serv,
	unspec
};

struct URL_Parsed
{	
	char *addr;
	char *port;
	char *file;
	Persistent pers;
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
