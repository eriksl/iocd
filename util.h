#ifndef _util_h_
#define _util_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <poll.h>
#include <stdint.h>

class Util
{
	public:

		typedef vector<uint8_t> byte_array;

		static int	parse_bytes(string str, byte_array & values)					throw();
		static int	timespec_diff(timespec from, timespec to)						throw();
};
#endif
