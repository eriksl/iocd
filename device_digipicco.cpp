#include "device_digipicco.h"
#include "control_digipicco.h"
#include "syslog.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

DeviceDigipicco::DeviceDigipicco(Devices *parent_devices, const Identity &id_in,
			int address_in) throw(exception)
	:
		DeviceI2C(parent_devices, id_in, address_in)
{
	stringstream conv;

	conv << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_shortname(conv.str());

	conv.str("");
	conv << "Digipicco digital temperature and humidity meter at " << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_longname(conv.str());

	if(!_probe())
		throw(minor_exception(string("no digipicco device found at ") + _shortname));
}

DeviceDigipicco::~DeviceDigipicco() throw()
{
}

bool DeviceDigipicco::_probe() throw()
{
	stringstream			conv;
	Util::byte_array		in;
	int						attempt;
	ControlDigipicco		*control_hum = 0;
	ControlDigipicco		*control_temp = 0;

	try
	{
		for(attempt = 0; attempt < 3; attempt++)
		{
			in = command("r 04 ");

			if(in.size() == 4)
				break;

			usleep(100000);
		}

		if(attempt > 2)
			throw(minor_exception("incorrect length in reply"));

		if((in[0] == 0xff) && (in[1] == 0xff) && (in[2] == 0xff) && (in[3] == 0xff))
			throw(minor_exception("incorrect reply"));

		control_hum = new ControlDigipicco(&_controls, Identity(_generation + 1, _id, _enumerator, path()),
										0, 100, "%", 0, ControlDigipicco::control_humidity);
		_enumerator++;
		control_temp = new ControlDigipicco(&_controls, Identity(_generation + 1, _id, _enumerator, path()),
										-40, 125, "˙C", 2, ControlDigipicco::control_temperature);
		_enumerator++;
	}
	catch(minor_exception e)
	{
		dlog("digipicco not found: %s\n", e.message.c_str());
	}

	if(!control_hum || !control_temp)
		return(false);

	dlog("probe: digipicco detected\n");

	_controls.add(control_hum);
	_controls.add(control_temp);

	return(true);
}
