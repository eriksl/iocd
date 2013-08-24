#include "interface.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

Interface::Interface(Interfaces *root_in, ID id_in) throw(exception)
	:
		id(id_in),
		root(root_in),
		enumerator(1)
{
	pthread_mutex_init(&mutex, 0);
}

Interface::~Interface() throw()
{
	devices.clear();
	pthread_mutex_destroy(&mutex);
}

Devices* Interface::interface_devices() throw()
{
	return(&devices);
}

void Interface::lock() throw(exception)
{
	pthread_mutex_lock(&mutex);
}

void Interface::unlock() throw(exception)
{
	pthread_mutex_unlock(&mutex);
}
