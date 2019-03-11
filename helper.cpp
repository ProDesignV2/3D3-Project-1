// Helper functions definitions

#include "helper.h"

#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

void *
get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET){
		// Return IPV4 address
		struct sockaddr_in  *sa_in = (struct sockaddr_in *)sa;
		return &(sa_in->sin_addr);
	}
	// Return IPV6 address
	struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)sa;
	return &(sa_in6->sin6_addr);
}

unsigned short int
get_in_port(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET){
		// Return IPV4 port
		struct sockaddr_in *sa_in = (struct sockaddr_in *)sa;
		return sa_in->sin_port;
	}
	// Return IPV6 port
	struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)sa;
	return sa_in6->sin6_port;
}

int
send_all(int send_fd, const char *buf, int *len)
{
	int num_bytes, bytes_left = *len, bytes_sent = 0;

	while(bytes_sent < *len){
		if((num_bytes = send(send_fd, buf + bytes_sent, bytes_left, 0)) == -1){ break; }
		bytes_sent += num_bytes;
		bytes_left -= num_bytes;
	}
	
	// Put number of sent bytes into original length variable
	*len = bytes_sent;

	// Return success outcome
	return num_bytes == -1 ? -1 : 0;
}

URL_Parsed
parse_url(char *url)
{
	// Parse URL into address, port and file path
	URL_Parsed purl;
	char *addr, *port, *file;
	int addrl, portl, filel;
	char *end = url + strlen(url);
	printf("Hello\n");
	if((addr = std::find(url, end, '/') + 2) == end){ perror("url : host"); exit(0); }
	if((file = std::find(addr, end, '/') + 1) == end){ perror("url : port"); exit(1); }
	if((port = std::find(addr, file - 2, ':') + 1) == file - 2){ perror("url : path"); exit(2); }
	addrl = port - addr - 1;
	portl = file - port - 1;
	filel = end - file + 1;
	purl.addr = new char[addrl];
	purl.port = new char[portl];
	purl.file = new char[filel];
	memcpy(purl.addr, addr, addrl);
	memcpy(purl.port, port, portl);
	memcpy(purl.file, file, filel);
	return purl;
}
