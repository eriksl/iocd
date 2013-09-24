#ifndef _exception_h_
#define _exception_h_

#include <string>
using std::string;

#include <exception>
using std::exception;

class iocd_exception : public exception
{
	public:

		string	message;
		iocd_exception(string msg) throw() : message(msg) {}
		virtual	~iocd_exception() throw() {}
};

class minor_exception : public iocd_exception
{
	public:

		minor_exception(string msg) throw() : iocd_exception(msg) {}
};

class major_exception : public iocd_exception
{
	public:

		major_exception(string msg) throw() : iocd_exception(msg) {}
};

class fatal_exception : public iocd_exception
{
	public:

		fatal_exception(string msg) throw() : iocd_exception(msg) {}
};

class oneshot_exception : public iocd_exception
{
	public:

		oneshot_exception() throw() : iocd_exception("") {}
};

#endif
