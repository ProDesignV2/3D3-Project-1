#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#define SERVER_PORT "40000"
#define BACKLOG 10

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
main()
{
	struct addrinfo hints, *server_info, *res_point;
	int server_fd, client_fd;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;
	int yes = 1, gai_result;
	char ipstr[INET6_ADDRSTRLEN] = {'\0'};

	// Set parameters for address structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// Get available address structs based on host IP
	if((gai_result = getaddrinfo(NULL, SERVER_PORT, &hints, &server_info)) != 0){
		fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(gai_result));
		return 1;
	}
	
	// Loop through getaddrinfo results for available struct  
	for(res_point = server_info; res_point != NULL; res_point = res_point->ai_next){
		
		// Create a main socket for the server using TCP IP (Stream)
		if((server_fd = socket(res_point->ai_family, res_point->ai_socktype, res_point->ai_protocol)) == -1) {
			perror("server : socket");
			continue;
		}		

		// Allow others to reuse the socket
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			// Why is this an exit call ?
			exit(1);
		}

		// Bind host address to main socket
		if (bind(server_fd, res_point->ai_addr, res_point->ai_addrlen) == -1) {
			close(server_fd);
			perror("server : bind");
			continue;
		}

		break;
	}

	// Free addrinfo struct - Should res_point be freed too?
	freeaddrinfo(server_info);	
	
	// If no working address struct is found
	if(res_point == NULL){
		fprintf(stderr, "server : failed to bind\n");
		exit(1);
	} 
	
	// Set server socket to listen status
	if (listen(server_fd, BACKLOG) == -1) {
		perror("server : listen");
		// Why is this an exit call ?
		exit(1);
	}
	
	// Reap all dead processes here ?
	
	printf("server : waiting for connections...\n");

	// Wait for  connections and deal with them
	while(1){

		// Accept a new connection
		client_addr_size = sizeof client_addr;
		printf("server : accepting...\n");
		if((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size)) == -1){
			perror("accept");
			continue;
		}
		
		// Print client address and port details
		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), ipstr, sizeof ipstr);
		std::cout << "server : accepted a connection from " << ipstr << ":" <<
		ntohs(get_in_port((struct sockaddr *)&client_addr)) << std::endl;

		// read/write data from/into the connection
		bool isEnd = false;
		char buf[20] = {0};
		std::stringstream ss;

		while (!isEnd) {
			memset(buf, '\0', sizeof(buf));

			if (recv(client_fd, buf, 20, 0) == -1) {
			      perror("recv");
			      return 5;
			}

			ss << buf << std::endl;
			std::cout << buf << std::endl;


			if (send(client_fd, buf, 20, 0) == -1) {
			      perror("send");
			      return 6;
			}

			if (ss.str() == "close\n")
			      break;

			ss.str("");
			
		}

		close(client_fd);
	}
	
	return 0;
}
