#include "device_tsl2550.h"
#include "control_tsl2550.h"
#include "devices.h"
#include "syslog.h"
#include "cppstreams.h"

DeviceTSL2550::DeviceTSL2550(Devices *parent_devices,
		const Identity &id_in, int address_in) throw(exception)
	:
		Device(parent_devices, id_in),
			_address(address_in)
{
	stringstream conv;

	conv << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_shortname(conv.str());

	conv.str("");
	conv << "tsl2550 light sensor at " << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_longname(conv.str());

	if(!_probe())
		throw(minor_exception(string("no tsl2550 device found at ") + _shortname));
}

DeviceTSL2550::~DeviceTSL2550() throw()
{
}

bool DeviceTSL2550::_probe() throw()
{
	stringstream			conv;
	Interface::byte_array	out;
	ControlTSL2550			*control = 0;

	try
	{
		// s <72> p w 00 p w 03 p r 01 p // cycle power ensure fresh readings

		out = command("w 00 p w 03 p r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if(out[0] != 0x03)
			throw(minor_exception("incorrect reply from probe"));

		control = new ControlTSL2550(&_controls, _generation + 1, _id, _enumerator, path(),
										0, 6500, "lux", 2);
	}
	catch(minor_exception e)
	{
		dlog("tsl2550 not found: %s\n", e.message.c_str());
	}

	if(!control)
		return(false);

	dlog("probe: tsl2550 detected\n");

	_enumerator++;
	_controls.add(control);

	return(true);
}

Interface::byte_array DeviceTSL2550::command(string cmd, int timeout, int chunks) const throw(exception)
{
	stringstream			in;
	string					out;
	Interface::byte_array	bytes;

	dlog("DeviceTSL2550::command: address = %x/%x\n", _address, _address << 1);

	in << "s " << hex << setfill('0') << setw(2) << (_address << 1) << " p " << cmd << " p ";
	out = _devices->interface()->command(in.str(), timeout, chunks);
	Interface::parse_bytes(out, bytes);

	return(bytes);
}
