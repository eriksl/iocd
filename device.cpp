#include "device.h"
#include "devices.h"
#include "controls.h"
#include "cppstreams.h"

Device::Device(Devices *parent_devices, const Identity& id_in) throw(exception)
	:	Identity(id_in),
			_devices(parent_devices),
			_controls(this),
			_enumerator(1)
{
}

Device::~Device() throw()
{
}

Devices* Device::devices() throw()
{
	return(_devices);
}

Controls* Device::controls() throw()
{
	return(&_controls);
}
