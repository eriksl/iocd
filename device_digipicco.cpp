#include "device_digipicco.h"
#include "control_digipicco.h"
#include "devices.h"
#include "syslog.h"
#include "cppstreams.h"

#include <unistd.h>

DeviceDigipicco::DeviceDigipicco(Devices *parent_devices,
			int generation_in, int parent_id_in, int ordinal_in,
			string parent_path_in, int address) throw(exception)
	:
		Device(parent_devices, generation_in, parent_id_in, ordinal_in, parent_path_in),
		_address(address)
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
	stringstream				conv;
	Interface::byte_array		in;
	int							attempt;
	ControlDigipicco			*control_hum = 0;
	ControlDigipicco			*control_temp = 0;

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

		control_hum = new ControlDigipicco(&_controls, _generation + 1, _id, _enumerator, path(),
										0, 100, "%", 0, ControlDigipicco::control_humidity);
		_enumerator++;
		control_temp = new ControlDigipicco(&_controls, _generation + 1, _id, _enumerator, path(),
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

Interface::byte_array DeviceDigipicco::command(string cmd, int timeout, int chunks) const throw(exception)
{
	stringstream			in;
	string					out;
	Interface::byte_array	bytes;

	dlog("DeviceDigipicco::command: address = %x/%x\n", _address, _address << 1);

	in << "s " << hex << setfill('0') << setw(2) << (_address << 1) << " p " << cmd << " p ";
	out = _devices->interface()->command(in.str(), timeout, chunks);
	Interface::parse_bytes(out, bytes);

	return(bytes);
}
