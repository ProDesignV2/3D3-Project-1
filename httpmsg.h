#ifndef HTTPMSG_H
#define HTTPMSG_H

// Header file for declaring HTTP Request and Response messages

#include <string>
#include <ctime>

#define MSG_TIMEOUT 5

class
HTTP_Message
{
	protected:
		std::string msg;
		bool has_body;
		size_t body_length;
		clock_t timeout;
	public:
		HTTP_Message();
	        bool append(char *buf, int n_bytes);
		void add_header(std::string header_line);
		const char *get_msg();
		int len_msg();
		bool reset_timeout();
};

class
HTTP_Request : public HTTP_Message
{
	public:
		HTTP_Request();
		HTTP_Request(char *buf, int n_bytes);
		std::string get_filename(bool client, std::string dir);
};

class
HTTP_Response : public HTTP_Message
{
	public:
		HTTP_Response();
		HTTP_Response(char *buf, int n_bytes);
		void add_body(std::string file_path);
		void save_body(std::string file_path);
};

#endif
