#ifndef _exception_h_
#define _exception_h_

#include <string>
using std::string;

#include <exception>
using std::exception;

class minor_exception : public exception
{
	public:

		string	message;
		minor_exception(string msg) : message(msg)
		{
		}
		virtual	~minor_exception() throw()
		{
		}
};

class major_exception : public exception
{
	public:

		string	message;
		major_exception(string msg) : message(msg)
		{
		}
		virtual	~major_exception() throw()
		{
		}
};

class fatal_exception : public exception
{
	public:

		string	message;
		fatal_exception(string msg) : message(msg)
		{
		}
		virtual	~fatal_exception() throw()
		{
		}
};

#endif
