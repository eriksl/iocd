#include "devices.h"
#include "syslog.h"

Devices::Devices(Interface * interface_in) throw(string) : _interface(interface_in)
{
}

Devices::~Devices() throw()
{
	iterator_t it;

	for(it = _devices.begin(); it != _devices.end(); it++)
		delete *it;

	_devices.clear();
}

Devices::const_iterator_t Devices::begin() const throw()
{
	return(_devices.begin());
}

Devices::const_iterator_t Devices::end() const throw()
{
	return(_devices.end());
}

Interface* Devices::interface() const throw()
{
	return(_interface);
}

void Devices::add(Device * device_in) throw()
{
	_devices.push_back(device_in);
}
