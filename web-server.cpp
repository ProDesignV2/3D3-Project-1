#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/stat.h>
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

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "4000"
#define DEFAULT_FILEDIR "."
#define BACKLOG 10
#define BUFFER_SIZE 1024

int
main(int argc, char *argv[])
{
	struct addrinfo hints, *server_info, *res_point;
	int listener_fd, client_fd, highest_fd, gai_result, n_bytes, closed_fd, yes = 1;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;
	char ipstr[INET6_ADDRSTRLEN], buf[BUFFER_SIZE];
	fd_set master, readfds;
	std::list<int> master_list;	
	std::string server_hostname, server_port, server_filedir;	
	
	server_hostname = DEFAULT_HOST;
	server_port = DEFAULT_PORT;
	server_filedir = DEFAULT_FILEDIR;

	// Set command arguments as server details
	switch(argc){ 
		case 1: break;
		case 2: server_hostname = argv[1]; break;
		case 3: server_hostname = argv[1]; server_port = argv[2]; break;
		case 4: server_hostname = argv[1]; server_port = argv[2]; server_filedir = argv[3]; break;
		default:		
			fprintf(stderr, "usage: ./web-server [hostname] [port] [file-dir]\n"); 
			exit(0); 
	}

	// Set parameters for address structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	// hints.ai_flags = AI_PASSIVE;

	// Get available address structs based on host IP
	if((gai_result = getaddrinfo(server_hostname.c_str(),
				     server_port.c_str(),
			 	     &hints,
			   	     &server_info)) != 0){
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
			n_bytes = 0;	
			
			// Create empty HTTP request
			HTTP_Request req;

			while(!(req.append(buf, n_bytes))){
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
					break;
				}
			}
			
			if(curr_client_fd == closed_fd){ continue; }

			// Create HTTP response
			HTTP_Response resp;

			if(req.is_error()){ resp.add_header("HTTP/1.0 400 Bad request"); }
			else{
				// Parse HTTP request	
				std::string client_path = req.get_filename(false, server_filedir);
				if(client_path.compare("./ ")){ client_path = "./index.html"; }	
				std::cout << client_path << std::endl;
				struct stat file_buffer;

				// Check filename
				if(stat (client_path.c_str(), &file_buffer) != 0)
				{ resp.add_header("HTTP/1.0 404 Not found"); }
				else{
					resp.add_header("HTTP/1.0 200 OK");
					resp.add_header("Content-Encoding: binary");
					resp.add_header("Content-Type: text/plain");
					resp.add_body(client_path);
				}
			}

			n_bytes = resp.len_msg();

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
			
			// Set new socket to non-blocking, is it automatically set by listener ?
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
