#include "util.h"
#include "cppstreams.h"

#include <errno.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

bool Util::isdaemon	= false;
bool Util::debug	= false;

void Util::vlog(const char * format, ...) throw()
{
    va_list ap;

    va_start(ap, format);

    if(isdaemon)
        vsyslog(LOG_WARNING, format, ap);
    else
        vfprintf(stderr, format, ap);

    va_end(ap);
}

void Util::dlog(const char * format, ...) throw()
{
    va_list ap;

	if(debug)
	{
    	va_start(ap, format);

    	if(isdaemon)
        	vsyslog(LOG_DEBUG, format, ap);
    	else
        	vfprintf(stderr, format, ap);
	}

    va_end(ap);
}

int Util::parse_bytes(string str, ByteArray & values) throw()
{
	stringstream	conv;
	int				byte;

	conv.str(str);

	for(;;)
	{
		conv >> hex >> byte;

		if(conv.eof() || conv.fail())
			break;

		values.push_back((uint8_t)(byte & 0xff));
	}

	return(values.size());
}

int Util::timespec_diff(timespec from, timespec to) throw()
{
	timespec temp;

	if((to.tv_nsec - from.tv_nsec) < 0)
	{
		temp.tv_sec		= to.tv_sec - from.tv_sec - 1;
		temp.tv_nsec	= 1000000000 + to.tv_nsec - from.tv_nsec;
	}
	else
	{
		temp.tv_sec		= to.tv_sec - from.tv_sec;
		temp.tv_nsec	= to.tv_nsec - from.tv_nsec;
	}

	return((temp.tv_sec * 1000) + (temp.tv_nsec / 1000000));
}

string Util::int_to_string(int in) throw()
{
	stringstream conv;
	conv << in;
	return(conv.str());
}

string Util::hex_to_string(int in, int width) throw()
{
	stringstream conv;
	conv << setw(width) << setfill('0') << hex << in;
	return(conv.str());
}

string Util::float_to_string(double value, int precision) throw()
{
	stringstream conv;
	conv << fixed << setprecision(precision) << value;
	return(conv.str());
}

int Util::string_to_int(string in) throw()
{
	stringstream conv;
	int rv;

	conv.str(in);
	conv >> rv;
	return(rv);
}

string Util::remove_newlines(string in) throw()
{
	string::iterator	it;
	string				rv;

	for(it = in.begin(); it != in.end(); it++)
		if((*it != '\n') && (*it != '\r'))
			rv += *it;

	return(rv);
}

string Util::usb_error_string(ssize_t in) throw()
{
	string rv;
//#ifdef TARGET_MIPSEL
//	(void)in;
//	rv = "unspecified libusb error";
//#else
	rv = libusb_error_name(in);
//#endif

	return(rv);
}

void ByteArray::from_memory(size_t length, const uint8_t *memory) throw()
{
	while(length-- > 0)
		push_back(*memory++);
}

uint8_t *ByteArray::to_memory(size_t *length) const throw()
{
	vector<uint8_t>::const_iterator it;
	uint8_t *base	= new uint8_t[size()];
	uint8_t *ptr = base;

	*length = 0;

	for(it = begin(); it != end(); it++)
	{
		(*length)++;
		*(ptr++) = *it;
	}

	return(base);
}

string ByteArray::to_hexstring() const throw()
{
	string	rv;
	ByteArray::const_iterator it;

	for(it = begin(); it != end(); it++)
	{
		if(it != begin())
			rv += " ";
		rv += Util::hex_to_string(*it, 2);
	}

	return(rv);
}

ByteArray::operator string() const throw()
{
	return(string("[") + Util::int_to_string(size()) + "] " + to_hexstring());
}
