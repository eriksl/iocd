#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>

#include <sstream>
#include <iomanip>
using namespace std;

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "http_server.h"
#include "syslog.h"

int HttpServer::page_dispatcher_root(MHD_Connection * connection, const string & method, ConnectionData * con_cls, const KeyValues &) const throw()
{
	string			data;
	string			value;
	ostringstream	conv;

	con_cls += 0; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	try
	{
		data += "<table>";
		data += "</table>\n";
	}
	catch(string e)
	{
		return(http_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, e));
	}
	catch(...)
	{
		return(http_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Unknown error"));
	}

	return(send_html(connection, "/", MHD_HTTP_OK, data));
}

int HttpServer::page_dispatcher_stylecss(MHD_Connection * connection, const string & method, ConnectionData * con_cls, const KeyValues &) const throw()
{
	string data;

	con_cls += 0; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	data += "\n"
"table	{\n"
"			border: 2px outset #eee;\n"
"			background-color: #ddd;\n"
"			text-align: center;\n"
"}\n"
"td.title {\n"
"			border: 2px inset #eee;\n"
"			background-color: #ddd;\n"
"			margin: 0px 0px 0px 0px;\n"
"			text-align: center;\n"
"			font-weight: bold;\n"
"}\n"
"td.heading {\n"
"			border: 2px inset #eee;\n"
"			background-color: #ddd;\n"
"			margin: 0px 0px 0px 0px;\n"
"			text-align: center;\n"
"			font-weight: bold;\n"
"}\n"
"td.l {\n"
"			border: 2px inset #eee;\n"
"			background-color: #ddd;\n"
"			margin: 0px 0px 0px 0px;\n"
"			text-align: left;\n"
"}\n"
"td {\n"
"			border: 2px inset #eee;\n"
"			background-color: #ddd;\n"
"			margin: 0px 0px 0px 0px;\n"
"			text-align: right;\n"
"}\n"
".input {\n"
"			width: 40px;\n"
"			border: 2px inset #eee;\n"
"			background-color: #eee;\n"
"			margin: 0px 0px 0px 0px;\n"
"			text-align: right;\n"
"}\n"
;
	return(send_raw(connection, MHD_HTTP_OK, data, "text/css"));
}
