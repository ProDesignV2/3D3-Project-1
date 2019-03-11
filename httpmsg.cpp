#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>

#include "httpmsg.h"

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

HTTP_Request::HTTP_Request(char *buf)
{
	// Convert buffer into HTTP request message
	msg = buf;
}

std::string
HTTP_Request::get_path()
{
	// The 10 represents the space for GET http://
	unsigned int start_path = msg.find("/", 11);
	unsigned int end_path = msg.find(" HTTP");
	std::string path = msg.substr(start_path, end_path - start_path);
	path.insert(0, ".");
	return path;
}

HTTP_Response::HTTP_Response(char *buf)
{
	// Convert buffer into HTTP response message
	msg = buf;
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
