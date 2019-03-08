#include <fstream>
#include <iterator>
#include <vector>

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
