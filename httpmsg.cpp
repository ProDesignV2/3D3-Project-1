#include <httpmsg.h>
#include <fstream>
#include <iterator>
#include <vector>

HTTP_Message::HTTP_Message() : msg("") {}

HTTP_Request::HTTP_Request() : msg("") {}

HTTP_Response::HTTP_Response() : msg("") {}

void 
HTTP_Message::add_header(string header_line)
{
	msg.append(header_line);
	msg.append("\r\n");
}

string
HTTP_Message::get_message()
{
	msg.append("\r\n");
	return msg;
}

string
HTTP_Response::get_message()
{
	return msg;
}

void
HTTP_Response::add_body(string file_path)
{	
	// Append file to msg as binary file
	std::ifstream input(file_path, std::ios::binary);
	std::vector<unsigned char> buffer((std::istreambuf_iterator<unsigned char>(input)),
					  (std::istreambuf_iterator<unsigned char>());
	// Add char vector to string
}
