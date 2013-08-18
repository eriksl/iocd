#ifndef _util_h_
#define _util_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <poll.h>
#include <stdint.h>
#include <unistd.h>

class ByteArray : public vector<uint8_t>
{
	public:

		void	from_memory(size_t length, const uint8_t *data)	throw();
		uint8_t	*to_memory(size_t *length) const throw();

				operator string() const throw();
};

class Util
{
	public:

		static	bool	isdaemon;
		static	bool	debug;

		static void		vlog(const char * format, ...)					throw();
		static void		dlog(const char * format, ...)					throw();
		static int		parse_bytes(string str, ByteArray & values)		throw();
		static int		timespec_diff(timespec from, timespec to)		throw();
		static string	int_to_string(int)								throw();
		static string	hex_to_string(int, int width = 4)				throw();
		static int		string_to_int(string)							throw();
		static string	float_to_string(double, int)					throw();
		static string	remove_newlines(string)							throw();
		static string	usb_error_string(ssize_t in)					throw();
};
#endif
