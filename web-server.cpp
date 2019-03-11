#include <sys/types.h>
// #include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <string>

#include "httpmsg.h"
#include "helper.h"

#define SERVER_PORT "9876"
#define BACKLOG 10
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
*/

int
main()
{
	struct addrinfo hints, *server_info, *res_point;
	int listener_fd, client_fd, highest_fd, gai_result, n_bytes, closed_fd, yes = 1;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;
	char ipstr[INET6_ADDRSTRLEN], buf[BUFFER_SIZE];
	fd_set master, readfds;
	std::list<int> master_list;	

	// Set parameters for address structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// Get available address structs based on host IP
	if((gai_result = getaddrinfo(NULL, SERVER_PORT, &hints, &server_info)) != 0){
		fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(gai_result));
		exit(0);
	}
	
	// Loop through getaddrinfo results for available struct  
	for(res_point = server_info; res_point != NULL; res_point = res_point->ai_next){
		
		// Create a main socket for the server using TCP IP (Stream)
		if((listener_fd = socket(res_point->ai_family, res_point->ai_socktype, res_point->ai_protocol)) == -1) {
			perror("server : socket");
			continue;
		}		

		// Set listener socket to non-blocking
		fcntl(listener_fd, F_SETFL, O_NONBLOCK);

		// Allow others to reuse the socket
		if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			// Why is this an exit call ?
			exit(1);
		}

		// Bind host address to main socket
		if (bind(listener_fd, res_point->ai_addr, res_point->ai_addrlen) == -1) {
			close(listener_fd);
			perror("server : bind");
			continue;
		}

		break;
	}

	// Free addrinfo struct - Should res_point be freed too ?
	freeaddrinfo(server_info);	
	
	// If no working address struct is found
	if(res_point == NULL){
		fprintf(stderr, "server : failed to bind\n");
		exit(2);
	} 
	
	// Set server socket to listen status
	if (listen(listener_fd, BACKLOG) == -1) {
		perror("server : listen");
		// Why is this an exit call ?
		exit(3);
	}
	
	// Reap all dead processes here ?
	
	// Set highest file descriptor	
	highest_fd = listener_fd;	
	
	// Clear both sets and add listener socket to master
	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(listener_fd, &master);

	// Set socket removal flag
	closed_fd = -1;
	
	printf("server : waiting for connections...\n");

	// Wait for  connections and deal with them
	while(1){
		
		// Copy master into temporary readfds
		readfds = master;	
		
		// Check if read set has available sockets	
		if(select(highest_fd + 1, &readfds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
			
		// Iterate through client sockets and check for send/receive
		for(const auto& curr_client_fd : master_list){
			
			// Not available for reading
			if(!FD_ISSET(curr_client_fd, &readfds)){ continue; }

			memset(&buf, 0, sizeof buf);
			
			if((n_bytes = recv(curr_client_fd, buf, sizeof buf, 0)) <= 0) {
			      	if(n_bytes == 0){
					printf("server : socket %d hung up\n", curr_client_fd);
				}
				else{ perror("recv"); }
				// Close server side of connection
			      	close(curr_client_fd);
				FD_CLR(curr_client_fd, &master);
				// Flag removal of socket
				closed_fd = curr_client_fd;
				continue;
			}

			std::cout << buf;
			
			// Try to send .txt file back
			HTTP_Response resp;
			resp.add_header("HTTP/1.0 200 OK");
			resp.add_header("Content-Encoding: binary");
			resp.add_header("Content-Type: text/plain");
			resp.add_header("Content-Length: 999");
			n_bytes = resp.len_msg() + 1;

			HTTP_Request req(buf);
			std::string test = req.get_path();
			std::cout << test << std::endl;
			resp.add_body(req.get_path());
			
			if (send_all(curr_client_fd, resp.get_msg(), &n_bytes) == -1) {
			      	perror("send_all");
			      	continue;
			}
		}		
	
		if(closed_fd != -1){
			// Remove socket from list and update highest FD
			master_list.remove(closed_fd);
			if(closed_fd == highest_fd){
				highest_fd = listener_fd;
				for(const auto& update_fd : master_list){
					if(update_fd > highest_fd){ highest_fd = update_fd; }
				}
			}		
			closed_fd = -1;
		}	
			
		// Check to see if new connections are available
		if(FD_ISSET(listener_fd, &readfds)){
			
			// Accept a new connection
			client_addr_size = sizeof client_addr;
			printf("server : accepting...\n");
			if((client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &client_addr_size)) == -1){
				perror("accept");
				continue;
			}
			
			// Set new socket to non-blocking
			fcntl(client_fd, F_SETFL, O_NONBLOCK);
	
			// Add new client socket to list and set
			master_list.push_back(client_fd);
			FD_SET(client_fd, &master);

			// Update highest file descriptor
			if(client_fd > highest_fd){ highest_fd = client_fd; }
			
			// Print client address and port details
			inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), ipstr, sizeof ipstr);
			std::cout << "server : accepted a connection from " << ipstr << ":" <<
			ntohs(get_in_port((struct sockaddr *)&client_addr)) << std::endl;
		}
	}
	
	return 0;
}
