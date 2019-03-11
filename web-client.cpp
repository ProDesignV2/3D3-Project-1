#include <sys/types.h>
// #include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "httpmsg.h"
#include "helper.h"

#define BUFFER_SIZE 1024

/*
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

char *
parse_url(char *url)
{
	//
	char *address, *port_num, *file_path;
	char *end = url + sizeof(url) / sizeof(url[0]);
	char *addr, *port, *file;
	addr = std::find(url, end, '/') + 2;
	port = std::find(addr, end, ':') + 1;
	file = std::find(port, end, '/');
	address =  
	return;
}
*/

int
main(int argc, char *argv[])
{
	struct addrinfo hints, *server_info, *res_point;
	int sock_fd, gai_result, n_bytes = 14;
	char ipstr[INET6_ADDRSTRLEN], buf[BUFFER_SIZE] = "Hello World!\n";
	
	// Set command line argument as address
	if(argc < 2){
		fprintf(stderr, "usage: ./web-client [URL]\n");
		exit(0);
	}

	// Set parameters for address structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Parse address and port
	URL_Parsed purl = parse_url(argv[1]);
	printf("addr [%s]\nport [%s]\nfile [%s]\n", purl.addr, purl.port, purl.file);

	// Get available address structs based on host IP
	if((gai_result = getaddrinfo("localhost", "9876", &hints, &server_info)) != 0){
		fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(gai_result));
		exit(1);
	}
	
	// Loop through getaddrinfo results for available struct  
	for(res_point = server_info; res_point != NULL; res_point = res_point->ai_next){
		
		// Create a socket for the server using TCP IP (Stream)
		if((sock_fd = socket(res_point->ai_family, res_point->ai_socktype, res_point->ai_protocol)) == -1) {
			perror("client : socket");
			continue;
		}		

		// Connect to server
		if (connect(sock_fd, res_point->ai_addr, res_point->ai_addrlen) == -1) {
			close(sock_fd);
			perror("client : connect");
			continue;
		}

		break;
	}

	// If no working address struct is found
	if(res_point == NULL){
		fprintf(stderr, "client : failed to connect\n");
		exit(2);
	} 
	
	// Reap all dead processes here for client ?

	// Print server address and port details
	inet_ntop(res_point->ai_family, get_in_addr(res_point->ai_addr), ipstr, sizeof ipstr);
	std::cout << "client : connecting to " << ipstr << ":" <<
	ntohs(get_in_port(res_point->ai_addr)) << std::endl;

	// Free addrinfo struct - Should res_point be freed too ?
	freeaddrinfo(server_info);		

	// Send HTTP request for file
	// Receive all of HTTP response with file in body
	// Parse file and save to local folder

	HTTP_Request req;
	req.add_header("GET http://localhost:9876/test.html HTTP/1.0");
 	n_bytes = req.len_msg();		
	
	if (send_all(sock_fd, req.get_msg(), &n_bytes) == -1) {
		perror("send_all");
		exit(4);
	}

	if((n_bytes = recv(sock_fd, buf, BUFFER_SIZE - 1, 0)) <= 0) {
		perror("recv");
		exit(3);
	}

	HTTP_Response resp(buf);
	resp.save_body("./test3.html");
		
	for(int c = 0; c < n_bytes; c++){
		// if(buf[c] < 32 || buf[c] > 126){ buf[c] = '#'; }
	}

	buf[n_bytes] = '\0';

	std::cout << resp.len_msg() << std::endl << buf;

	close(sock_fd);

	return 0;
}
