#ifndef HTTPMSG_H
#define HTTPMSG_H

// Header file for declaring HTTP Request and Response messages

#include <string>

class
HTTP_Message
{
	protected:
		std::string msg;
		bool has_body;
	public:
		HTTP_Message();
		void add_header(std::string header_line);
		const char *get_msg();
		int len_msg();
};

class
HTTP_Request : public HTTP_Message
{
	public:
		HTTP_Request();
};

class
HTTP_Response : public HTTP_Message
{
	public:
		HTTP_Response();
		void add_body(std::string file_path);
};

#endif
