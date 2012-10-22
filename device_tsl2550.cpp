#include "device_tsl2550.h"
#include "control_tsl2550.h"
#include "devices.h"
#include "syslog.h"
#include "cppstreams.h"
#include "util.h"

DeviceTSL2550::DeviceTSL2550(Devices *parent_devices,
		const Identity &id_in, int address_in) throw(exception)
	:
		DeviceI2C(parent_devices, id_in, address_in)
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
	stringstream		conv;
	Util::byte_array	out;
	ControlTSL2550		*control = 0;

	try
	{
		// s <72> p w 00 p w 03 p r 01 p // cycle power ensure fresh readings

		out = command("w 00 p w 03 p r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if(out[0] != 0x03)
			throw(minor_exception("incorrect reply from probe"));

		control = new ControlTSL2550(&_controls, Identity(_generation + 1, _id, _enumerator, path()),
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
