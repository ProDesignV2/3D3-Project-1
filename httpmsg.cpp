#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <string.h>

#include "httpmsg.h"
#include "helper.h"

HTTP_Message::HTTP_Message() : msg(""), has_body(false) {}

HTTP_Request::HTTP_Request() : HTTP_Message() {}

HTTP_Response::HTTP_Response() : HTTP_Message() {}

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

HTTP_Request::HTTP_Request(char *buf, int n_bytes)
{
	// Convert buffer into HTTP request message
	msg = std::string(buf, n_bytes);
}

bool
HTTP_Request::append(char *buf, int n_bytes)
{
    // Append  buffer to  HTTP request message
    msg.append(std::string(buf, n_bytes));
    // Check if completed request and return
    //
}

std::string
HTTP_Request::get_path(bool client)
{
	// The 4 represents the space for GET
	size_t end_path = msg.find(" HTTP");
	char *temp = new char[end_path - 4];
	strcpy(temp, msg.substr(4, end_path - 4).c_str());
	URL_Parsed purl = parse_url(temp);
	std::string path(purl.file);
	if(client){ 
		path = path.substr(path.find_last_of("/") + 1);		
        path.insert(0, "wow");
	}
	else{
		path.insert(0, ".");
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
	msg.append("\r\n");
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
