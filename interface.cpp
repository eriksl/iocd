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
	Util::dlog("II interface destructor start\n");

	if(fd >= 0)
		::close(fd);

	pthread_mutex_destroy(&mutex);

	Util::dlog("II interface destructor end\n");
}

void Interface::command(void *cmd) throw(exception)
{
	string result;

	try
	{
		lock();
		interface_command(cmd);
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
