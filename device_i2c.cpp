#include "device_i2c.h"
#include "cppstreams.h"
#include "util.h"

DeviceI2C::DeviceI2C(Devices *parent_devices, const Identity& id_in, int address_in) throw(exception)
	:	Device(parent_devices, id_in),
			_address(address_in)
{
}

DeviceI2C::~DeviceI2C() throw()
{
}

Util::byte_array DeviceI2C::command(string cmd, int timeout, int chunks) throw(exception)
{
	stringstream		in;
	Util::byte_array	bytes;

	in << "s " << hex << setfill('0') << setw(2) << (_address << 1) << " p " << cmd << " p";
	bytes = Device::command(in.str(), timeout, chunks);

	return(bytes);
}
