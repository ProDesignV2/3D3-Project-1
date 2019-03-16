#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <string.h>
#include <sstream>
#include <ctime>

#include "httpmsg.h"
#include "helper.h"

HTTP_Message::HTTP_Message() : msg(""), has_body(false), body_length(0), timeout(0) {}

HTTP_Request::HTTP_Request() : HTTP_Message() {}

HTTP_Response::HTTP_Response() : HTTP_Message() {}

bool
HTTP_Message::append(char *buf, int n_bytes)
{	
	double duration = (std::clock() - timeout) / (double) CLOCKS_PER_SEC;
	// Start timer if not already started
	if(timeout == 0){ timeout = std::clock(); }
	else if(duration >= MSG_TIMEOUT){ return true; }
	// Append  buffer to  HTTP request message
	msg.append(std::string(buf, n_bytes));
	// Check if completed request and return
	if(body_length == 0){
		// Check for end of header
		size_t header_end = msg.find("\r\n\r\n");
		if(header_end != std::string::npos){
			// Need to check for body if Content-Length is present
			size_t content_start = msg.find("Content-Length: ");
			// If not present, message is request
			if(content_start == std::string::npos){ timeout = 0; return true; }
			content_start += 16;
			size_t content_end = msg.find("\r\n", content_start) - 1;
			std::string content_length = msg.substr(content_start,
								content_end - content_start + 1);
			std::stringstream parse_len(content_length);
 			parse_len >> body_length;
		}	
	}	
	if((msg.find("\r\n\r\n") + 4 + body_length) == msg.length()){
		timeout = 0;
		return true;
	}
	return false;
}

void 
HTTP_Message::add_header(std::string header_line)
{
	msg.append(header_line);
	msg.append("\r\n");
}

const char *
HTTP_Message::get_msg()
{
	if(!has_body){ msg.append("\r\n"); }
	return msg.c_str();
}

int
HTTP_Message::len_msg()
{
	return has_body ? msg.length() : msg.length() + 2;
}

bool
HTTP_Message:: reset_timeout()
{
	if(timeout == 0){ return false; }
	timeout = 0;
	return true;	
}

HTTP_Request::HTTP_Request(char *buf, int n_bytes)
{
	// Convert buffer into HTTP request message
	msg = std::string(buf, n_bytes);
}

std::string
HTTP_Request::get_filename(bool client, std::string dir)
{
	// The 4 represents the space for GET
	size_t len_path = msg.find(" HTTP") - 4;
	char *temp = new char[len_path + 1];
	strcpy(temp, msg.substr(4, len_path).c_str());
	URL_Parsed purl = parse_url(temp);
	std::string path(purl.file);
	if(client){
		path = path.substr(path.find_last_of("/") + 1);		
		path.insert(0, "wow");
	}
	else{
		path.insert(0, dir);
	}
	return path;
}

HTTP_Response::HTTP_Response(char *buf, int n_bytes)
{
	// Convert buffer into HTTP response message
	msg = std::string(buf, n_bytes);
}

void
HTTP_Response::add_body(std::string file_path)
{	
	// Read into char vector as binary file
	std::ifstream input(file_path, std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

	// Add char vector to message string
	std::string body(buffer.begin(), buffer.end());
	body_length = body.length();
	msg.append("Content-Length: ");
	std::stringstream ss;
	ss << body_length;
	msg.append(ss.str());
	msg.append("\r\n\r\n");
	msg.append(body);
	has_body = true;
}

void
HTTP_Response::save_body(std::string file_path)
{	
	// Write string out to binary file
	std::ofstream output(file_path, std::ios::binary);
	unsigned int body_index = msg.find("\r\n\r\n") + 4;
	output << msg.substr(body_index);
}
