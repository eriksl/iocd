#include "devices.h"
#include "device.h"
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

Device* Devices::find(string id) throw(string)
{
	Devices::iterator device;

	if(id.length() == 8)
		id = id.substr(2, 2);

	if(id.length() != 2)
		throw(string("find(device): id has invalid length"));

	for(device = begin(); device != end(); device++)
		if((**device).id().substr(2,2) == id)
			break;

	if(device == end())
		throw(string("find(device): device not found"));

	return(*device);
}
