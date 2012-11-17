#include "devices.h"
#include "device.h"
#include "id.h"

Devices::Devices() throw(exception)
{
}

Devices::~Devices() throw()
{
	clear();
}

Devices::iterator Devices::begin() throw()
{
	return(devices.begin());
}

Devices::iterator Devices::end() throw()
{
	return(devices.end());
}

void Devices::clear() throw()
{
	iterator it;

	for(it = devices.begin(); it != devices.end(); it++)
		delete it->second;

	devices.clear();
}

void Devices::add(Device *device) throw()
{
	devices[device->id] = device;
}

Device* Devices::find_device(ID id) throw(exception)
{
	Devices::iterator it;

	id.control_type		= 0;
	id.control_index	= 0;

	if((it = devices.find(id)) == devices.end())
		throw(minor_exception("DS find_device: device not found"));

	return(it->second);
}
