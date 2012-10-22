#include "device.h"
#include "devices.h"
#include "controls.h"
#include "interface.h"

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

Util::byte_array Device::command(string cmd, int timeout, int chunks) throw(exception)
{
	string				out;
	Util::byte_array	bytes;

	out = _devices->interface()->command(cmd, timeout, chunks);
	Util::parse_bytes(out, bytes);

	return(bytes);
}
