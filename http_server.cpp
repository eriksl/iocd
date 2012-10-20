#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

#include "http_server.h"
#include "syslog.h"
#include "cppstreams.h"

HttpServer::HttpServer(Interfaces * interfaces_in, int tcp_port, bool multithread_in) throw(string)
	: interfaces(interfaces_in), multithread(multithread_in)
{
	page_dispatcher_map["/"]				=	&HttpServer::page_dispatcher_root;
	page_dispatcher_map["/read"]			=	&HttpServer::page_dispatcher_read;
	page_dispatcher_map["/write"]			=	&HttpServer::page_dispatcher_write;
	page_dispatcher_map["/readcounter"]		=	&HttpServer::page_dispatcher_readcounter;
	page_dispatcher_map["/resetcounter"]	=	&HttpServer::page_dispatcher_resetcounter;
	page_dispatcher_map["/readpwmmode"]		=	&HttpServer::page_dispatcher_readpwmmode;
	page_dispatcher_map["/writepwmmode"]	=	&HttpServer::page_dispatcher_writepwmmode;
	page_dispatcher_map["/info"]			=	&HttpServer::page_dispatcher_info;
	page_dispatcher_map["/style.css"]		=	&HttpServer::page_dispatcher_stylecss;

	dlog("start HttpServer\n");

	daemon_handle_ipv4 = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
			tcp_port, 0, 0, &HttpServer::access_handler_callback, this,
			MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::callback_request_completed, this,
			MHD_OPTION_END);

	page_dispatcher_map["/"]			=	&HttpServer::page_dispatcher_root;
	page_dispatcher_map["/"]			=	&HttpServer::page_dispatcher_stylecss;

	daemon = MHD_start_daemon(multithread_option | MHD_USE_IPv6 | MHD_USE_DEBUG,
			tcp_port, 0, 0, &HttpServer::access_handler_callback, this,
			MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::callback_request_completed, this,
			MHD_OPTION_END);

	if(daemon == 0)
		throw(string("Cannot start http daemon"));
}

HttpServer::~HttpServer() throw(string)
{
	MHD_stop_daemon(daemon);
	daemon = 0;
}

void HttpServer::poll(int timeout) throw(string)
{
	if(multithread)
	{
		if(timeout < 0)
			for(;;)
				sleep(65536);
		else
			usleep(timeout);
	}
	else
	{
		fd_set				read_fd_set, write_fd_set, except_fd_set;
		int					max_fd = 0;
		struct timeval		tv;
		struct timeval *	tvp;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&except_fd_set);

		if(MHD_get_fdset(daemon, &read_fd_set, &write_fd_set, &except_fd_set, &max_fd) == MHD_NO)
			throw(string("error in MHD_get_fdset"));

		if(timeout >= 0)
		{
			tv.tv_sec	= timeout / 1000000;
			tv.tv_usec	= (timeout % 1000000);
			tvp = &tv;
		}
		else
			tvp = 0;

		if(select(max_fd + 1, &read_fd_set, &write_fd_set, &except_fd_set, tvp) != 0)
			MHD_run(daemon);
	}
}

string HttpServer::html_header(const string & title, int reload, string reload_url, string cssurl)
{
	stringstream	ss;
	string			refresh_header;

	if(reload)
	{
		ss << reload;
		refresh_header = "        <meta http-equiv=\"Refresh\" content=\"" + ss.str();

		if(reload_url.size() != 0)
			refresh_header += ";url=" + reload_url;

		refresh_header += "\"/>\n";
	}

	if(cssurl != "")
		cssurl = "<link rel=\"stylesheet\" href=\"/style.css\"/>";

	return(string("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n") +
				"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n" +
				"    <head>\n" +
				"        <meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\"/>\n" +
				cssurl +
				refresh_header + "\n" +
				"        <title>" + title + "</title>\n" + 
				"    </head>\n" +
				"    <body>\n");
}

string HttpServer::html_footer()
{
	return(string("    </body>\n") +
				"</html>\n");
}

int HttpServer::send_raw(MHD_Connection * connection, int http_code,
			const string & data, const string & data_mime,
			const string & cookie_id, const string & cookie_value) const throw(string)
{
	int						rv;
	struct MHD_Response	*	response;
	void *					data_copy = strdup(data.c_str());

	response = MHD_create_response_from_data(data.size(), data_copy, MHD_NO, MHD_YES);
	MHD_add_response_header(response, "Content-Type", data_mime.c_str());

	if(cookie_id.size())
	{
		string cookie = cookie_id + "=" + cookie_value + "; path=/;";
		MHD_add_response_header(response, "Set-Cookie", cookie.c_str());
	}

	rv = MHD_queue_response(connection, http_code, response);
	MHD_destroy_response(response);

	free(data_copy);

	return(rv);
}

int HttpServer::send_html(MHD_Connection * connection, const string & title, int http_code,
			const string & message, int reload, const string & reload_url,
			const string & cookie_id, const string & cookie_value) const throw(string)
{
	string					data;

	data = html_header(title, reload, reload_url, "/style.css");
	data += message;
	data += html_footer();

	return(send_raw(connection, http_code, data, "text/html", cookie_id, cookie_value));
}

int HttpServer::http_error(MHD_Connection * connection, int http_code, const string & message) const throw(string)
{
	return(send_html(connection, "ERROR", http_code, string("<p>") + message + "</p>\n"));
}

int HttpServer::access_handler_callback(void * void_http_server,
		struct MHD_Connection * connection,
		const char * url, const char * method, const char * version,
		const char * upload_data, size_t * upload_data_size,
		void ** con_cls)
{
	HttpServer * http_server = (HttpServer *)void_http_server;

	if(*con_cls == 0)
	{
		ConnectionData * ncd = new(ConnectionData);
		ncd->callback_count = 0;
		ncd->postprocessor	= MHD_create_post_processor(connection, 1024, callback_postdata_iterator, ncd);
		*con_cls = (void *)ncd;
	}
	else
		(**(ConnectionData **)con_cls).callback_count++;

	if(string(method) == "POST")
	{
		if((**(ConnectionData **)con_cls).callback_count == 0)
			return(MHD_YES);
	}

	if(*upload_data_size)
	{
		MHD_post_process((**(ConnectionData **)con_cls).postprocessor, upload_data, *upload_data_size);
		*upload_data_size = 0;
		return(MHD_YES);
	}

	return(http_server->access_handler(connection,
		url, method, version, *(ConnectionData **)con_cls, upload_data_size, upload_data));
};

int HttpServer::access_handler(struct MHD_Connection * connection,
		const string & url, const string & method, const string &,
		ConnectionData * con_cls, size_t *, const char *) const
{
	PageHandler::map_t::const_iterator	it;
	PageHandler::dispatcher_function_t	fn;

	for(it = page_dispatcher_map.begin(); it != page_dispatcher_map.end(); it++)
		if(it->first == string(url)) 
			break;

	if(it != page_dispatcher_map.end())
	{
		fn = it->second;

		KeyValues variables;

		if(method == "POST")
			variables = con_cls->values;

		if((method == "GET") || (method == "POST"))
		{
			KeyValues post_arguments = get_http_values(connection, MHD_GET_ARGUMENT_KIND);
			variables.data.insert(post_arguments.data.begin(), post_arguments.data.end());
		}

		return((this->*fn)(connection, method, con_cls, variables));
	}

	return(http_error(connection, MHD_HTTP_NOT_FOUND, string("ERROR: URI ") + url + " not found"));
}

int HttpServer::callback_keyvalue_iterator(void * cls, enum MHD_ValueKind, const char * key, const char * value)
{
	KeyValues * rv = (KeyValues *)cls;

	rv->data[string(key)] = string(value);

	return(MHD_YES);
}

HttpServer::KeyValues HttpServer::get_http_values(struct MHD_Connection * connection, enum MHD_ValueKind kind) const
{
	KeyValues rv;

	MHD_get_connection_values(connection, kind, callback_keyvalue_iterator, &rv);

	return(rv);
}

void * HttpServer::callback_request_completed(void *, struct MHD_Connection *, void ** con_cls, enum MHD_RequestTerminationCode)
{
	if(con_cls && *con_cls)
	{
		ConnectionData * cdp = (ConnectionData *)*con_cls;

		if(cdp->postprocessor)
		{
			MHD_destroy_post_processor(cdp->postprocessor);
			cdp->postprocessor = 0;
		}

		delete(cdp);
		*con_cls = 0;
	}

	return(0);
}

int HttpServer::callback_postdata_iterator(void * con_cls, enum MHD_ValueKind,
		const char * key, const char *, const char *,
		const char *, const char * data, uint64_t, size_t size)
{
	string mangle;
	ConnectionData * condata = (ConnectionData *)con_cls;

	mangle.append(data, size);
	condata->values.data[key] = mangle;
	return(MHD_YES);
}

string HttpServer::KeyValues::dump(bool html) const
{
	string rv;
	map<string, string>::const_iterator it;

	if(html)
		rv = "<table border=\"1\" cellspacing=\"0\" cellpadding=\"0\">\n";

	for(it = data.begin(); it != data.end(); it++)
	{
		if(html)
			rv += "<tr><td>\n";

		rv += it->first;

		if(html)
			rv += "</td><td>\n";
		else
			rv += " = ";

		rv += it->second;

		if(html)
			rv += "</td></tr>\n";
		else
			rv += "\n";
	}

	if(html)
		rv += "</table>\n";

	return(rv);
}

//	based on javascript encodeURIComponent()
//	http://www.zedwood.com/article/111/cpp-urlencode-function	

static string char2hex(char dec)
{
	char	dig1	= (dec & 0xf0) >> 4;
	char	dig2	= (dec & 0x0f);
	string	result;

	if(0 <= dig1 && dig1 <= 9)
		dig1 += 48;			//0,48 inascii
	if(10 <= dig1 && dig1 <= 15)
		dig1 += 97 - 10;	//a,97 inascii
	if(0 <= dig2 && dig2 <= 9)
		dig2 += 48;
	if(10 <= dig2 && dig2 <= 15)
		dig2 += 97 - 10;

	result.append(&dig1, 1);
	result.append(&dig2, 1);
	return(result);
}

string HttpServer::uriencode(string in) throw()
{
	string	escaped = "";
	int		ix, length;

	length = in.length();

	for(ix = 0; ix < length; ix++)
	{
		if((48 <= in[ix] && in[ix] <= 57)	||			//	0 - 9
			(65 <= in[ix] && in[ix] <= 90)	||			//	abc - xyz
			(97 <= in[ix] && in[ix] <= 122)	||			//	ABC - XYZ
			(in[ix]=='~' || in[ix]=='!' || in[ix]=='*' || in[ix]=='(' || in[ix]==')' || in[ix]=='\''))
		{
			escaped.append(&in[ix], 1);
		}
		else
		{
			escaped.append("%");
			escaped.append(char2hex(in[ix]));			//converts char 255 to string "ff"
		}
	}
	return(escaped);
}
