#ifndef _util_h_
#define _util_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <poll.h>
#include <stdint.h>
#include <unistd.h>

class Util
{
	public:

		typedef vector<uint8_t> byte_array;

		static	bool	isdaemon;
		static	bool	debug;

		static void		vlog(const char * format, ...)					throw();
		static void		dlog(const char * format, ...)					throw();
		static int		parse_bytes(string str, byte_array & values)	throw();
		static int		timespec_diff(timespec from, timespec to)		throw();
		static string	int_to_string(int)								throw();
		static int		string_to_int(string)							throw();
		static string	float_to_string(double, int)					throw();
		static string	remove_newlines(string)							throw();
		static string	usb_error_string(ssize_t in)					throw();
};
#endif
