#include "device_atmel.h"
#include "control_atmel.h"
#include "devices.h"
#include "syslog.h"
#include "cppstreams.h"

#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <math.h>

DeviceAtmel::DeviceAtmel(Devices *parent_devices,
			int generation_in, int parent_id_in, int ordinal_in,
			string parent_path_in, int address) throw(string)
	:
		Device(parent_devices, generation_in, parent_id_in, ordinal_in, parent_path_in),
		_address(address)
{
	stringstream conv;
	conv << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_shortname(conv.str());

	if(!_probe())
		throw(string("DeviceAtmel::DeviceAtmel: no device found at ") + _shortname);

	conv.str("");
	conv << "Atmel " << _modelname << "_" << _version << "." << _revision <<
			" at " << "i2c:0x" << hex << setfill('0') << setw(2) << _address;
	_set_longname(conv.str());
}

DeviceAtmel::~DeviceAtmel() throw()
{
}

Interface::byte_array DeviceAtmel::_getcontrol(int cmd) const throw(string)
{
	stringstream			in;
	Interface::byte_array	out;

	// w 07 xx 00: request controls

	in << "w 07 " << hex << setfill('0') << setw(2) << cmd;
	out = command(in.str());

	if(out.size() != 8)
		throw(string("DeviceAtmel::getcontrol: cmd read error"));

	if(out[1] != 0x07)
		throw(string("DeviceAtmel::getcontrol: device returns error code"));

	return(out);
}

bool DeviceAtmel::_probe() throw()
{
	stringstream			conv;
	Interface::byte_array	in;
	int						ix;

	try
	{
		// w 00: request identification
		in = command("w 00");

		if(in.size() != 24)
			throw(string("DeviceAtmel::_probe: invalid length"));

		if((in[1] != 0x00))
			throw(string("DeviceAtmel::_proble: device returns error code"));

		if((in[2] != 0x4a) || (in[3] != 0xfb))
			throw(string("device does not identify as atmel"));

		if((in[4] > 3) && (in[4] < 7))
			_model = in[4];
		else
			throw(string("unknown atmel model"));

		_version	= in[5];
		_revision	= in[6];

		for(ix = 7; ix < 24; ix++)
		{
			if(in[ix] == 0)
				break;
			_modelname += (char)in[ix];
		}

		dlog("probe: %s detected\n", _modelname.c_str());

		int						min, max;
		int						nrcontrols;
		ControlAtmel 			*control;

		in			= _getcontrol(0);
		nrcontrols	= in[2];

		dlog("probe: found %d digital inputs\n", nrcontrols);

		min = 0;
		max = (in[3] << 24) | (in[4] << 16) | (in[5] << 8) | in[6];

		for(ix = 0; ix < nrcontrols; ix++)
		{
			control = 0;

			try
			{
				control = new ControlAtmel(&_controls,
						2, _id, _enumerator, path(),
						min, max, "", 0, ControlAtmel::digital_input, ix);
			}
			catch(string(error))
			{
				dlog("add atmel digital input: %s\n", error.c_str());
			}

			if(control)
			{
				_enumerator++;
				_controls.add(control);
			}
		}

		in			= _getcontrol(1);
		nrcontrols	= in[2];

		dlog("probe: found %d analog inputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			control = 0;

			try
			{
				control = new ControlAtmel(&_controls,
						2, _id, _enumerator, path(),
						min, max, "", 0, ControlAtmel::analog_input, ix);
			}
			catch(string(error))
			{
				dlog("add atmel analog input: %s\n", error.c_str());
			}

			if(control)
			{
				_enumerator++;
				_controls.add(control);
			}
		}

		in			= _getcontrol(2);
		nrcontrols	= in[2];

		dlog("probe: found %d digital outputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			control = 0;

			try
			{
				control = new ControlAtmel(&_controls,
						2, _id, _enumerator, path(),
						min, max, "", 0, ControlAtmel::digital_output, ix);
			}
			catch(string(error))
			{
				dlog("add atmel digital output: %s\n", error.c_str());
			}

			if(control)
			{
				_enumerator++;
				_controls.add(control);
			}
		}

		in			= _getcontrol(3);
		nrcontrols	= in[2];

		dlog("probe: found %d pwm outputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			control = 0;

			try
			{
				control = new ControlAtmel(&_controls,
						2, _id, _enumerator, path(),
						min, max, "", 0, ControlAtmel::pwm_output, ix);
			}
			catch(string(error))
			{
				dlog("add atmel pwm output: %s\n", error.c_str());
			}

			if(control)
			{
				_enumerator++;
				_controls.add(control);
			}
		}
	}
	catch(string error)
	{
		dlog("DeviceAtmel::_probe: %s\n", error.c_str());
		return(false);
	}

	return(true);
}

Interface::byte_array DeviceAtmel::command(string cmd, int timeout, int chunks) const throw(string)
{
	stringstream			in;
	string					out;
	int						ix;
	uint8_t					checksum;
	Interface::byte_array	bytes;
	int						length;

	in << "s " << hex << setfill('0') << setw(2) << (_address << 1) << " p " << cmd << " p r 00 p ";
	out = _devices->interface()->command(in.str(), timeout, chunks);

	length = Interface::parse_bytes(out, bytes);

	if(length < 3)
		throw(string("DeviceAtmel::command: received too little bytes"));

	checksum = 0;
	for(ix = 0; ix < length - 1; ix++)
		checksum += bytes[ix];

	if(checksum != bytes[length - 1])
		throw(string("DeviceAtmel::command: checksum error"));

	if(bytes[0] != 0x00)
		throw(string("DeviceAtmel::command: device returns error code"));

	return(bytes);
}
