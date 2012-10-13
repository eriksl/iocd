#include "device.h"
#include "devices.h"
#include "controls.h"

Device::Device(Interface *parent_interface, Devices *parent_devices) throw(string) :
	_interface(parent_interface),
	_devices(parent_devices)
{
	_controls = new Controls(parent_interface, this);
}

Device::~Device() throw()
{
	delete _controls;
}

string Device::name() const throw()
{
	return(_name);
}

string Device::bus() const throw()
{
	return(_bus);
}

Interface* Device::interface() const throw()
{
	return(_interface);
}

Devices* Device::devices() const throw()
{
	return(_devices);
}

Controls* Device::controls() const throw()
{
	return(_controls);
}
