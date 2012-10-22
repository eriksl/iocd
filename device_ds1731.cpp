#include "device_ds1731.h"
#include "control_ds1731.h"
#include "devices.h"
#include "syslog.h"
#include "cppstreams.h"
#include "util.h"

DeviceDS1731::DeviceDS1731(Devices *parent_devices,
			const Identity &id_in, int address_in) throw(exception)
	:
		Device(parent_devices, id_in),
			_address(address_in)
{
	stringstream conv;

	conv << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_shortname(conv.str());

	conv.str("");
	conv << "ds1731 temperature sensor at " << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_longname(conv.str());

	if(!_probe())
		throw(minor_exception(string("no ds1731 device found at ") + _shortname));
}

DeviceDS1731::~DeviceDS1731() throw()
{
}

bool DeviceDS1731::_probe() throw()
{
	stringstream			conv;
	Util::byte_array	out;
	ControlDS1731			*control = 0;

	try
	{
		// "w 54 p" // Software POR (cycle power)

		out = command("w 54 r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		// "w 22 p" // Stop Convert T (stop measuring continiously)

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		out = command("w 22 r 01");

		// "w ac r 01 p" Access Config (check config register)

		out = command("w ac r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if((out[0] & 0xfc) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w ac 8c r 01 p" Access Config (put device in 12 bits / continuous mode)

		out = command("w ac 8c r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if((out[0] & 0xef) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w 51 p" Start Convert T (start convert)

		out = command("w 51 r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		control = new ControlDS1731(&_controls, _generation + 1, _id, _enumerator, path(),
										-55, 125, "˙C", 2);
	}
	catch(minor_exception e)
	{
		dlog("ds1731 not found: %s\n", e.message.c_str());
	}

	if(!control)
		return(false);

	dlog("probe: ds1731 detected\n");

	_enumerator++;
	_controls.add(control);

	return(true);
}

Util::byte_array DeviceDS1731::command(string cmd, int timeout, int chunks) const throw(exception)
{
	stringstream			in;
	string					out;
	Util::byte_array	bytes;

	dlog("DeviceDS1731::command: address = %x/%x\n", _address, _address << 1);

	in << "s " << hex << setfill('0') << setw(2) << (_address << 1) << " p " << cmd << " p ";
	out = _devices->interface()->command(in.str(), timeout, chunks);
	Util::parse_bytes(out, bytes);

	return(bytes);
}
