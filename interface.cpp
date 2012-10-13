#include "interface.h"
#include "devices.h"
#include "cppstreams.h"
#include "syslog.h"

Interface::Interface(Interfaces * parent_interfaces, const string &id) throw(string) :
	_id(id), _fd(-1), _interfaces(parent_interfaces)
{
	_devices		= new Devices(this);
	_mutex_valid	= false;
	pthread_mutex_init(&_mutex, 0);
	_mutex_valid	= true;
}

Interface::~Interface() throw()
{
	if(_mutex_valid)
	{
		pthread_mutex_destroy(&_mutex);
		_mutex_valid = false;
	}

	delete _devices;
}

void Interface::lock() throw(string)
{
	if(_mutex_valid)
		pthread_mutex_lock(&_mutex);
	else
		throw(string("Interface::lock: mutex invalid"));
}

void Interface::unlock() throw(string)
{
	if(_mutex_valid)
		pthread_mutex_unlock(&_mutex);
	else
		throw(string("Interface::unlock: mutex invalid"));
}

Interfaces* Interface::interfaces() const throw()
{
	return(_interfaces);
}

Devices* Interface::devices() const throw()
{
	return(_devices);
}

int Interface::parse_bytes(string str, byte_array & values) throw()
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

int Interface::timespec_diff(timespec from, timespec to) throw()
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
