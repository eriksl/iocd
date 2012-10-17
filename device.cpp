#include "device.h"
#include "devices.h"
#include "controls.h"
#include "cppstreams.h"

Device::Device(Devices *parent_devices, int generation_in, int parent_id_in, int ordinal_in, string parent_path_in) throw(string)
	:	Identity(generation_in, parent_id_in, ordinal_in, parent_path_in),
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
