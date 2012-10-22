#include "device_tmp275.h"
#include "control_tmp275.h"
#include "syslog.h"
#include "cppstreams.h"
#include "util.h"

DeviceTMP275::DeviceTMP275(Devices *parent_devices,
			const Identity &id_in, int address_in) throw(exception)
	:
		DeviceI2C(parent_devices, id_in, address_in)
{
	stringstream conv;

	conv << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_shortname(conv.str());

	conv.str("");
	conv << "TMP275 digital thermometer at " << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_longname(conv.str());

	if(!_probe())
		throw(minor_exception(string("no tmp275 device found at ") + _shortname));
}

DeviceTMP275::~DeviceTMP275() throw()
{
}

bool DeviceTMP275::_probe() throw()
{
	stringstream		conv;
	Util::byte_array	in;
	ControlTMP275		*control = 0;

	try
	{
		// write 0xabcd in threshold register,
		// check treshold register, it should return 0xabc0

		in = command("w 02 ab cd r 02");

		if(in.size() != 2)
			throw(minor_exception("incorrect length in reply"));

		if((in[0] != 0xab) || (in[1] != 0xc0))
			throw(minor_exception("incorrect reply from probe"));

		in = command("w 01 e1 r 01");

		if(in.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if(in[0] != 0x61)
			throw(minor_exception("incorrect reply from probe"));

		control = new ControlTMP275(&_controls, Identity(_generation + 1, _id, _enumerator, path()),
										-40, 125, "˙C", 2);
	}
	catch(minor_exception e)
	{
		dlog("tmp275 not found: %s\n", e.message.c_str());
	}

	if(!control)
		return(false);

	dlog("probe: tmp275 detected\n");

	_enumerator++;
	_controls.add(control);

	return(true);
}
