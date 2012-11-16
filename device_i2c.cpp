#include "interface_elv.h"
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
	InterfaceELV::cmd_t	cmd_elv;
	stringstream		in;
	Util::byte_array	bytes;

	in << "s " << hex << setfill('0') << setw(2) << (address << 1) << " p " << cmd << " p";

	cmd_elv.in		= in.str();
	cmd_elv.timeout = timeout;
	cmd_elv.chunks	= chunks;
	Device::command(&cmd_elv);

	Util::parse_bytes(cmd_elv.out, bytes);
	return(bytes);
}

string DeviceI2C::device_id() const throw()
{
	ostringstream rv;
	rv << name_short() << ":0x" << hex << setw(2) << setfill('0') << address;
	return(rv.str());
}
