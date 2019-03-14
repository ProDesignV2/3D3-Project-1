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
	std::string urlStr(url);
	size_t addr_pos = 0, port_pos, file_pos, url_len = urlStr.length();
	size_t addr_len, port_len, file_len;
	// Check for protocol
	if(urlStr.find("https://") != std::string::npos ||
	   urlStr.find("http://")  != std::string::npos   ){ addr_pos = urlStr.find("//") + 2; }
	// Check for port
	port_pos = urlStr.find(":", addr_pos);
	// Check for file
	file_pos = urlStr.find("/", addr_pos);
	// Get file length
	if(file_pos != std::string::npos){	
		if(port_pos != std::string::npos){
			addr_len = port_pos - addr_pos;
			port_len = file_pos - port_pos - 1;
			if(addr_pos == file_pos){ fprintf(stderr, "url : hostname\n"); exit(0); }
			if(port_len == 0){ fprintf(stderr, "url : port\n"); exit(0); }
		}
		else{
			addr_len = file_pos - addr_pos;
			port_len = 0;
			file_len = url_len - file_pos;
		}
		file_len = url_len - file_pos;
		if(addr_len == 0){ fprintf(stderr, "url : hostname\n"); exit(1); }
	}
	else{ fprintf(stderr, "url : filepath\n"); exit(2); }
	// Copy parsed URL parts into struct	
	purl.addr = new char[addr_len];
	purl.file = new char[file_len];
	strcpy(purl.addr, urlStr.substr(addr_pos, addr_len).c_str());
	strcpy(purl.file, urlStr.substr(file_pos, file_len).c_str());
	if(port_len == 0){ purl.port = NULL; return purl; }
	purl.port = new char[port_len];
	strcpy(purl.port, urlStr.substr(port_pos + 1, port_len).c_str());
	return purl;
}
