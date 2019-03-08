#ifndef HTTPMSG_H
#define HTTPMSG_H

// Header file for declaring HTTP Request and Response messages

#include <string>

class
HTTP_Message
{
	private:
		string msg;
	public:
		HTTP_Message();
		void add_header(string header_line);
		string get_message();
}

class
HTTP_Request : public HTTP_Message
{
	public:
		HTTP_Request();
}

class
HTTP_Response : public HTTP_Message
{
	public:
		HTTP_Response();
		string get_message();
		void add_body(string file_path);
}

#endif
