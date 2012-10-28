#include "device_i2c.h"
#include "cppstreams.h"
#include "util.h"

DeviceI2C::DeviceI2C(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:	Device(root_in, id_in),
			address(address_in)
{
}

Util::byte_array DeviceI2C::command(string cmd, int timeout, int chunks) throw(exception)
{
	stringstream		in;
	Util::byte_array	bytes;

	in << "s " << hex << setfill('0') << setw(2) << (address << 1) << " p " << cmd << " p";
	bytes = Device::command(in.str(), timeout, chunks);

	return(bytes);
}

string DeviceI2C::device_id() const throw()
{
	ostringstream rv;
	rv << name_short() << ":0x" << hex << setw(2) << setfill('0') << address;
	return(rv.str());
}
