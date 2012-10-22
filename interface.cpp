#include "interface.h"
#include "devices.h"
#include "identity.h"
#include "cppstreams.h"
#include "syslog.h"

#include "unistd.h"

Interface::Interface(Interfaces *parent_interfaces, const Identity &id_in) throw(exception)
	: Identity(id_in),
			_fd(-1),
			_enumerator(1),
			_devices(this),
			_interfaces(parent_interfaces)
{
	_mutex_valid	= false;
	pthread_mutex_init(&_mutex, 0);
	_mutex_valid	= true;
}

Interface::~Interface() throw()
{
	if(_fd >= 0)
		::close(_fd);

	if(_mutex_valid)
	{
		pthread_mutex_destroy(&_mutex);
		_mutex_valid = false;
	}
}

void Interface::_lock() throw(exception)
{
	//dlog("--> LOCK\n");

	if(_mutex_valid)
		pthread_mutex_lock(&_mutex);
	else
		throw(minor_exception("Interface::lock: mutex invalid"));
}

void Interface::_unlock() throw(exception)
{
	//dlog("<-- UNLOCK\n");

	if(_mutex_valid)
		pthread_mutex_unlock(&_mutex);
	else
		throw(minor_exception("Interface::unlock: mutex invalid"));
}

Interfaces* Interface::interfaces() const throw()
{
	return(_interfaces);
}

Devices* Interface::devices() throw()
{
	return(&_devices);
}

string Interface::command(string cmd, int timeout, int chunks) throw(exception)
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
