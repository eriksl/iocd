#include "interface.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

Interface::Interface(Interfaces *root_in, ID id_in) throw(exception)
	:
		id(id_in),
		root(root_in),
		fd(-1),
		enumerator(1)
{
	pthread_mutex_init(&mutex, 0);
}

Interface::~Interface() throw()
{
	if(fd >= 0)
		::close(fd);

	pthread_mutex_destroy(&mutex);
}

string Interface::command(string cmd, int timeout, int chunks) throw(exception)
{
	string result;

	try
	{
		lock();
		result = interface_command(cmd, timeout, chunks);
		unlock();
	}
	catch(...)
	{
		try
		{
			unlock();
		}
		catch(...)
		{
		}

		throw;
	}

	return(result);
}

Devices* Interface::interface_devices() throw()
{
	return(&devices);
}

void Interface::find_devices() throw(exception)
{
	throw(minor_exception("Device::find_devices called"));
}

void Interface::lock() throw(exception)
{
	pthread_mutex_lock(&mutex);
}

void Interface::unlock() throw(exception)
{
	pthread_mutex_unlock(&mutex);
}
