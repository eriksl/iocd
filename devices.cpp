#include "device.h"
#include "devices.h"
#include "identity.h"
#include "syslog.h"

Devices::Devices(Interface *interface_in) throw(string)
	: _interface(interface_in)
{
}

Devices::~Devices() throw()
{
	iterator it;

	for(it = _devices.begin(); it != _devices.end(); it++)
		delete *it;

	_devices.clear();
}

Devices::iterator Devices::begin() throw()
{
	return(_devices.begin());
}

Devices::iterator Devices::end() throw()
{
	return(_devices.end());
}

Interface* Devices::interface() throw()
{
	return(_interface);
}

void Devices::add(Device * device) throw()
{
	_devices.push_back(device);
}
