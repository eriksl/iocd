#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

#include "http_server.h"
#include "interfaces.h"
#include "interface.h"
#include "devices.h"
#include "device.h"
#include "controls.h"
#include "control.h"
#include "syslog.h"
#include "cppstreams.h"

static string write_form(string id) throw()
{
	string data;

	data += "<form method=\"get\" action=\"/write\">\n";
	data += "<div>\n";
	data += "<input type=\"text\" name=\"value\" />\n";
	data += "<input type=\"hidden\" name=\"control\" value=\"" + id + "\" />\n";
	data += "<input type=\"submit\" value=\"write\" />\n";
	data += "</div>\n";
	data += "</form>\n";

	return(data);
}

static string write_pwmmode_form(string id) throw()
{
	string data;

	data += "<form method=\"get\" action=\"/writepwmmode\">\n";
	data += "<div>\n";
	data += "<input type=\"text\" name=\"value\" />\n";
	data += "<input type=\"hidden\" name=\"control\" value=\"" + id + "\" />\n";
	data += "<input type=\"submit\" value=\"write\" />\n";
	data += "</div>\n";
	data += "</form>\n";

	return(data);
}

static string reset_form(string id) throw()
{
	string data;

	data += "<form method=\"get\" action=\"/resetcounter\">\n";
	data += "<div>\n";
	data += "<input type=\"hidden\" name=\"control\" value=\"" + id + "\" />\n";
	data += "<input type=\"submit\" value=\"reset\" />\n";
	data += "</div>\n";
	data += "</form>\n";

	return(data);
}

int HttpServer::page_dispatcher_root(MHD_Connection * connection, const string & method, ConnectionData * con_cls, const KeyValues &) const throw(exception)
{
	string			data;
	string			value;
	ostringstream	conv;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	try
	{
		Interfaces::iterator	interface;
		Devices::iterator		device;
		Controls::iterator		control;

		data += "<table>\n";
		data += "<tr><th>Interface</th><th>Device</th><th>Control</th><th>Value</th><th>Write value</th><th>Counter</th><th>Reset counter</th><th>Pwm mode</th><th>Write pwm mode</th></tr>\n";

		for(interface = _interfaces->begin(); interface != _interfaces->end(); interface++)
		{
			data += "<tr>\n";
			data += string("<td>") + (**interface).shortname().c_str() + "</td>\n";

			for(device = (**interface).devices()->begin(); device != (**interface).devices()->end(); device++)
			{
				if(device != (**interface).devices()->begin())
					data += "<tr><td/>";

				data += string("<td>") + (**device).shortname().c_str() + "</td>\n";

				for(control = (**device).controls()->begin(); control != (**device).controls()->end(); control++)
				{
					if(control != (**device).controls()->begin())
						data += "<tr><td/><td/>";

					data += string("<td><a href=\"/info?control=") + HttpServer::uriencode((**control).id()) + "\">" + (**control).longname().c_str() + "</a></td>";
					data += "<td>" + ((**control).canread() ? (**control).read_string() : "") + " " + (**control).unit() + "</td>";
					data += "<td>" + ((**control).canwrite() ? write_form((**control).id()) : "") + "</td>";
					data += "<td>" + ((**control).cancount() ? ((**control).readcounter_string()) : "") + "</td>";
					data += "<td>" + ((**control).cancount() ? reset_form((**control).id()) : "") + "</td>";
					data += "<td>" + ((**control).canpwm() ? (**control).readpwmmode_string() : "") + "</td>";
					data += "<td>" + ((**control).canpwm() ? write_pwmmode_form((**control).id()) : "") + "</td>";
					data += "</tr>\n";
				}
			}
		}

		data += "</table>\n";
	}
	catch(minor_exception e)
	{
		return(http_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, e.message));
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(...)
	{
		return(http_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Unknown error"));
	}

	return(send_html(connection, "/", MHD_HTTP_OK, data));
}

int HttpServer::page_dispatcher_read(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id;
	string								value;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		value = control->read_string();
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value + "] read &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "read", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_write(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id, value;
	int									intval;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	if((value = variables.data.find("value")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERROR: missing value"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		intval = lexical_cast<int>(value->second);
		control->write(intval);
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value->second + "] write &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "write", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_readcounter(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id;
	string								value;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		value = control->readcounter_string();
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value + "] readcounter &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "readcounter", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_resetcounter(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id;
	string								value;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		value = control->readresetcounter_string();
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value + "] resetcounter &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "resetcounter", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_readpwmmode(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id;
	string								value;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		value = control->readpwmmode_string();
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value + "] readpwmmode &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "readpwmmode", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_writepwmmode(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id, value;
	string								error;
	string								data;
	int									intval;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	if((value = variables.data.find("value")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERROR: missing value"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
		intval = lexical_cast<int>(value->second);
		control->writepwmmode(intval);
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(major_exception e)
	{
		throw;
	}
	catch(bad_lexical_cast e)
	{
		error = e.what();
	}
	catch(...)
	{
		error = "unknown error";
	}

	if(error.length() > 0)
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + error));

	data = "<p>OK [" + value->second + "] writepwmmode &lt;" + control->shortname() + "&gt;</p>\n";

	return(send_html(connection, "writepwmmode", MHD_HTTP_OK, data, 10, "/"));
}

int HttpServer::page_dispatcher_info(MHD_Connection *connection, const string &method, ConnectionData *con_cls, const KeyValues &variables) const throw(exception)
{
	Control								*control = 0;
	string_string_map::const_iterator	id;
	string								error;
	string								data;

	(void)con_cls; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "ERROR: method not allowed"));

	if((id = variables.data.find("control")) == variables.data.end())
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, "ERRROR: missing id"));

	try
	{
		control = _interfaces->find_control_by_name(id->second);
	}
	catch(minor_exception e)
	{
		return(http_error(connection, MHD_HTTP_BAD_REQUEST, string("ERROR: ") + e.message + " (" + id->second + ")"));
	}
	catch(major_exception e)
	{
		throw;
	}

	data += "<table><tr><th>Interface</th><th>Device</th><th>Id</th><th>Path</th><th>Short name</th><th>Long name</th>";
	data += "<th>Properties</th><th>Min</th><th>Max</th><th>Precision</th><th>Units</th><th></tr>\n";
	data += "<tr>\n";
	data += "<td>" + control->interface()->longname() + "</td>\n";
	data += "<td>" + control->device()->longname() + "</td>\n";
	data += "<td>" + control->id() + "</td>\n";
	data += "<td>" + control->path() + "</td>\n";
	data += "<td>" + control->shortname() + "</td>\n";
	data += "<td>" + control->longname() + "</td>\n";
	data += "<td>" + control->properties() + "</td>\n";
	data += "<td>" + control->min_string() + "</td>\n";
	data += "<td>" + control->max_string() + "</td>\n";
	data += "<td>" + control->precision_string() + "</td>\n";
	data += "<td>" + control->unit() + "</td>\n";
	data += "</tr>\n";
	data += "</table>\n";

	return(send_html(connection, "info", MHD_HTTP_OK, data, -1));
}

int HttpServer::page_dispatcher_restart(MHD_Connection *connection, const string &, ConnectionData *, const KeyValues &) const throw(exception)
{
	_interfaces->signal(Interfaces::signal_user_restart);

	return(send_html(connection, "restart", MHD_HTTP_OK, "<p>restart</p>", -1));
}

int HttpServer::page_dispatcher_quit(MHD_Connection *connection, const string &, ConnectionData *, const KeyValues &) const throw(exception)
{
	_interfaces->signal(Interfaces::signal_user_quit);

	return(send_html(connection, "restart", MHD_HTTP_OK, "<p>quit</p>", -1));
}

int HttpServer::page_dispatcher_stylecss(MHD_Connection * connection, const string & method, ConnectionData * con_cls, const KeyValues &) const throw(exception)
{
	string data;

	con_cls += 0; // ignore

	if(method != "GET" && method != "POST")
		return(http_error(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed"));

	data += "\n"
"table	{\n"
"			border: 2px inset #eee;\n"
"			background-color: #ddd;\n"
"}\n"
"td		{\n"
"			border: 1px solid #eee;\n"
"			background-color: #ddd;\n"
"}\n"
;
	return(send_raw(connection, MHD_HTTP_OK, data, "text/css"));
}
