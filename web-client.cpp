#include <sys/types.h>
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

#define BUFFER_SIZE 16384

int
main(int argc, char *argv[])
{
	struct addrinfo hints, *server_info, *res_point;
	int sock_fd, gai_result, n_bytes = 14;
	char ipstr[INET6_ADDRSTRLEN], buf[BUFFER_SIZE] = "Hello World!\n";
	
	// Set command line argument usage
	if(argc < 2){
		fprintf(stderr, "usage: ./web-client [URL] [URL] ...\n");
		exit(0);
	}

	// Cycle through requested URLs	
	int arg_curr = 1;
	while(arg_curr < argc){

		// Print current requested URL
		printf("%s\n", argv[arg_curr]);	

		// Parse address and port
		URL_Parsed purl = parse_url(argv[arg_curr]);

		// Set parameters for address structs
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		// Get available address structs based on host IP
		if((gai_result = getaddrinfo(purl.addr, purl.port, &hints, &server_info)) != 0){
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
		HTTP_Request req;
		std::string url(argv[arg_curr]);
		req.add_header("GET " + url + " HTTP/1.0");
		n_bytes = req.len_msg();		
	
        	// Send all of HTTP request    
		if (send_all(sock_fd, req.get_msg(), &n_bytes) == -1) {
			perror("send_all");
			exit(4);
		}
		
		// Create empty HTTP response
		HTTP_Response resp;
		n_bytes = 0;
	
		while(!(resp.append(buf, n_bytes))){
			// Receive HTTP response    
			if((n_bytes = recv(sock_fd, buf, BUFFER_SIZE, 0)) <= 0) {
				perror("recv");
				exit(3);
			}
		}

		if(resp.is_error()){
			fprintf(stderr, "client : recv timeout\n");
			exit(4);
		}
		
		// Check response for error code
		size_t resp_code = resp.get_code();
		
		if(resp_code == 400){ printf("400 Bad request\n"); }
		else if(resp_code == 404){ printf("404 Not found\n"); }
		else if(resp_code == 200){
        		// Parse response and save file to local folder
			std::string req_path = req.get_filename(true, "");
			if(req_path.compare("/")){ req_path = "new.html"; }
			resp.save_body(req_path);
		}
		else{ fprintf(stderr, "client : unknown error code\n"); exit(5); }

        	// Close current socket
		close(sock_fd);
        
       	 	// Go to next URL
		arg_curr++;
	}
	
	return 0;
}
