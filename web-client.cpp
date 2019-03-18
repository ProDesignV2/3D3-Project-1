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
#define DEFAULT_PORT "4000"

int
main(int argc, char *argv[])
{
	struct addrinfo hints, *server_info, *res_point;
	int sock_fd, gai_result, n_bytes = 14, n_urls;
	char ipstr[INET6_ADDRSTRLEN], buf[BUFFER_SIZE] = "Hello World!\n";
	size_t def_port_len = strlen(DEFAULT_PORT);	

	// Set command line argument usage
	if((n_urls = argc - 1) < 1){
		fprintf(stderr, "usage: ./web-client [URL] [URL] ...\n");
		exit(0);
	}

	// Load all URLs into array and parse
	URL_Parsed **urls = new URL_Parsed*[n_urls];
	for(int url_curr = 1; url_curr < argc; url_curr++){
		urls[url_curr - 1] = new URL_Parsed;
		*(urls[url_curr - 1]) = parse_url(argv[url_curr]);
		// Set default port if none specified
		if(urls[url_curr - 1]->port == NULL){ 
			urls[url_curr - 1]->port = new char[def_port_len];
			strcpy(urls[url_curr - 1]->port, DEFAULT_PORT);
		}
		// Set default connection if none specified
		if(urls[url_curr - 1]->pers == Persistent::unspec){
			urls[url_curr - 1]->pers = Persistent::pers;
		} 		
	}
		
	// Cycle through requested URLs	
	for(int url_curr = 0; url_curr < n_urls; url_curr++){
		
		// Load in current parse URL
		URL_Parsed *purl = urls[url_curr];

		// Check if current host domain has been served
		if(purl->pers == Persistent::pers_serv){ continue; }

		if(url_curr != 0){ printf("\n"); }
	
		// Print current requested host domain name
		printf("Host [%s:%s]\n", purl->addr, purl->port);

		// Set parameters for address structs
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		// Get available address structs based on host IP
		if((gai_result = getaddrinfo(purl->addr, purl->port, &hints, &server_info)) != 0){
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

		printf("Requested File(s)\n-----------------\n");

		// Check for other requests on same connection
		for(int host_curr = url_curr; host_curr < n_urls; host_curr++){
		
			// Check if any other requests for the same domain
			if(urls[host_curr]->pers == Persistent::pers_serv){ continue; }
			if(strcmp(urls[host_curr]->addr, purl->addr) != 0){ continue; }
			if(strcmp(urls[host_curr]->port, purl->port) != 0){ continue; }

			// Skip similar non-persistent hostnames
			if((host_curr != url_curr) && (urls[host_curr]->pers == Persistent::non_pers)){ continue; }
	
			// Print requested file
			printf(".%s ", urls[host_curr]->file);
		
			// Send HTTP request for file
			HTTP_Request req;
			std::string url(argv[host_curr + 1]);
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
			
			if(resp_code == 400){ printf("[400 Bad request]\n"); }
			else if(resp_code == 404){ printf("[404 Not found]\n"); }
			else if(resp_code == 200){
				printf("[200 OK]\n");
				// Parse response and save file to local folder
				std::string req_path = req.get_filename(true, "");
				if(req_path.compare("/") == 0){ req_path = "new.html"; }
				resp.save_body(req_path);
			}
			else{ fprintf(stderr, "client : unknown error code\n"); exit(5); }
				
			// Break if current URL connection is non-persistent
			if(urls[host_curr]->pers == Persistent::non_pers){ break; }
			
			// Flag that current URL has been served if persistent
			urls[host_curr]->pers = Persistent::pers_serv;	
		}

		// Close current socket
		close(sock_fd);
	}
	
	return 0;
}
