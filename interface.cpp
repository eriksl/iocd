#include "interface.h"
#include "devices.h"
#include "identity.h"
#include "cppstreams.h"
#include "syslog.h"

Interface::Interface(Interfaces *parent_interfaces,
			int generation_in, int parent_id_in, int ordinal_in,
			string parent_path_in, string) throw(string)
	:	Identity(generation_in, parent_id_in, ordinal_in, parent_path_in),
		_fd(-1), _enumerator(1), _devices(this), _interfaces(parent_interfaces)
{
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
}

void Interface::_lock() throw(string)
{
	dlog("--> LOCK\n");

	if(_mutex_valid)
		pthread_mutex_lock(&_mutex);
	else
		throw(string("Interface::lock: mutex invalid"));
}

void Interface::_unlock() throw(string)
{
	dlog("<-- UNLOCK\n");

	if(_mutex_valid)
		pthread_mutex_unlock(&_mutex);
	else
		throw(string("Interface::unlock: mutex invalid"));
}

Interfaces* Interface::interfaces() const throw()
{
	return(_interfaces);
}

Devices* Interface::devices() throw()
{
	return(&_devices);
}

string Interface::command(string cmd, int timeout, int chunks) throw(string)
{
	string result;

	try
	{
		_lock();
		result = _command(cmd, timeout, chunks);
		_unlock();
	}
	catch(...)
	{
		try
		{
			_unlock();
		}
		catch(...)
		{
		}

		throw;
	}

	return(result);
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
