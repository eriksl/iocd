#include "device_atmel.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

DeviceAtmel::DeviceAtmel(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:
		DeviceI2C(root_in, id_in, address_in)
{
	if(!probe())
		throw(minor_exception(string("no atmel device found at ") + Util::int_to_string(address)));
}

DeviceAtmel::~DeviceAtmel() throw()
{
}

string DeviceAtmel::name_short_static() throw()
{
	return(string("atmel"));
}

string DeviceAtmel::name_long_static() throw()
{
	return(string("Atmel ATtiny"));
}

string DeviceAtmel::name_short() const throw()
{
	return(name_short_static());
}

string DeviceAtmel::name_long() const throw()
{
	return(name_long_static());
}

double DeviceAtmel::read(Control *control) throw(exception)
{
	Util::byte_array bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = query(0x30, control->index, 4);
			return(double(bytes[2]));
		}

		case(DeviceAtmel::analog_input):
		{
			bytes = query(0xc0, control->index, 3);
			bytes = query(0x01, 0, 5);
			return(double((bytes[2] << 8) | (bytes[3])));
		}

		case(DeviceAtmel::digital_output):
		{
			bytes = query(0x50, control->index, 4);
			return(double(bytes[2]));
		}

		case(DeviceAtmel::pwm_output):
		{
			bytes = query(0x90, control->index, 5);
			return(double((bytes[2] << 8) | (bytes[3])));
		}

		default:
		{
			throw(minor_exception("DeviceAtmel::read: invalid operation for control"));
		}
	}

	return(-1);
}

void DeviceAtmel::write(Control *control, double value_in) throw(exception)
{
	int value = int(value_in);

	if((value < control->min) || (value > control->max))
		throw(minor_exception("write control: value out of range"));

	switch(control->type)
	{
		case(DeviceAtmel::digital_output):
		{
			query(0x40, control->index, 4, value & 0xff);
			break;
		}

		case(DeviceAtmel::pwm_output):
		{
			query(0x80, control->index, 3, (value & 0xff00) >> 8, value & 0x00ff);
			break;
		}

		default:
		{
			throw(minor_exception("DeviceAtmel::write: invalid operation for control"));
		}
	}
}

double DeviceAtmel::readwrite(Control *control, double value) throw(exception)
{
	double oldvalue = DeviceAtmel::read(control);
	DeviceAtmel::write(control, value);
	return(oldvalue);
}

int DeviceAtmel::readcounter(Control *control) throw(exception)
{
	Util::byte_array bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = query(0x10, control->index, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(minor_exception("DeviceAtmel::readcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int DeviceAtmel::readresetcounter(Control *control) throw(exception)
{
	Util::byte_array bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = query(0x20, control->index, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(minor_exception("DeviceAtmel::readresetcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int DeviceAtmel::readpwmmode(Control *control) throw(exception)
{
	Util::byte_array	bytes;
	int					mode;

	switch(control->type)
	{
		case(DeviceAtmel::digital_output):
		{
			bytes = query(0x70, control->index, 4);
			mode = bytes[2];

			if(mode == 4)
				mode = 3;

			return(mode);
		}

		case(DeviceAtmel::pwm_output):
		{
			bytes = query(0xb0, control->index, 4);
			mode = bytes[2];

			if(mode == 4)
				mode = 3;

			return(mode);
		}

		default:
		{
			throw(string("DeviceAtmel::readpwmmode: invalid operation for control"));
		}
	}

	return(-1);
}

void DeviceAtmel::writepwmmode(Control *control, int value) throw(exception)
{
	if((value < 0) || (value > 3))
		throw(minor_exception("write control: pwm mode out of range"));

	switch(control->type)
	{
		case(DeviceAtmel::digital_output):
		{
			query(0x60, control->index, 4, value & 0xff);
			break;
		}

		case(DeviceAtmel::pwm_output):
		{
			query(0xa0, control->index, 4, value & 0xff);
			break;
		}

		default:
		{
			throw(minor_exception("DeviceAtmel::writepwmmode: invalid operation for control"));
		}
	}
}

string DeviceAtmel::readpwmmode_string(Control *control) throw(exception)
{
	int mode = readpwmmode(control);

	switch(mode)
	{
		case(0): return("off");
		case(1): return("down");
		case(2): return("up");
		case(3): return("up/down");
		default: {}
	}

	return("invalid");
}

bool DeviceAtmel::probe() throw()
{
	stringstream		conv;
	Util::byte_array	in;
	int					ix;

	try
	{
		// w 00: request identification
		in = command("w 00");

		if(in.size() != 24)
			throw(minor_exception("DeviceAtmel::probe: invalid length"));

		if((in[1] != 0x00))
			throw(minor_exception("DeviceAtmel::_proble: device returns error code"));

		if((in[2] != 0x4a) || (in[3] != 0xfb))
			throw(minor_exception("device does not identify as atmel"));

		if((in[4] > 3) && (in[4] < 7))
			model = in[4];
		else
			throw(minor_exception("unknown atmel model"));

		version		= in[5];
		revision	= in[6];

		for(ix = 7; ix < 24; ix++)
		{
			if(in[ix] == 0)
				break;
			modelname += (char)in[ix];
		}
	}
	catch(minor_exception e)
	{
		Util::dlog("probe atmel: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe atmel: detected %s\n", modelname.c_str());
	return(true);
}

void DeviceAtmel::find_controls() throw()
{
	Util::byte_array	in;
	int					ix;
	int					min, max;
	int					nrcontrols;
	Control				*control;

	try
	{
		in			= getcontrol(0);
		nrcontrols	= in[2];
		control		= 0;

		Util::dlog("DD atmel: found %d digital inputs\n", nrcontrols);

		min = 0;
		max = (in[3] << 24) | (in[4] << 16) | (in[5] << 8) | in[6];

		for(ix = 0; ix < nrcontrols; ix++)
		{
			Control::capset cc;

			cc.set(Control::cap_isdigital);
			cc.set(Control::cap_canread);
			cc.set(Control::cap_cancount);

			control = new Control(root,
					ID(id.interface, id.device, DeviceAtmel::digital_input, ix + 1),
					min, max, "", 0, cc, DeviceAtmel::digital_input, ix,
					"din", "Digital input");

			if(control)
				controls.add(control);
		}
	}
	catch(minor_exception e)
	{
		Util::dlog("DD atmel: add digital input: %s\n", e.message.c_str());
	}

	try
	{
		in			= getcontrol(1);
		nrcontrols	= in[2];
		control		= 0;

		Util::dlog("DD atmel: found %d analog inputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			Control::capset cc;

			cc.set(Control::cap_canread);

			control = new Control(root,
					ID(id.interface, id.device, DeviceAtmel::analog_input, ix + 1),
					min, max, "", 0, cc, DeviceAtmel::analog_input, ix,
					"ain", "Analog input");
			if(control)
				controls.add(control);
		}
	}
	catch(minor_exception e)
	{
		Util::dlog("DD atmel: analog input: %s\n", e.message.c_str());
	}

	try
	{
		in			= getcontrol(2);
		nrcontrols	= in[2];
		control		= 0;

		Util::dlog("DD atmel: found %d digital outputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			Control::capset cc;

			cc.set(Control::cap_isdigital);
			cc.set(Control::cap_canread);
			cc.set(Control::cap_canwrite);
			cc.set(Control::cap_cansoftpwm);
			cc.set(Control::cap_canpwmmode);

			control = new Control(root,
					ID(id.interface, id.device, DeviceAtmel::digital_output, ix + 1),
					min, max, "", 0, cc, DeviceAtmel::digital_output, ix,
					"dout", "Digital output");

			if(control)
				controls.add(control);
		}
	}
	catch(minor_exception e)
	{
		Util::dlog("DD atmel: add digital output: %s\n", e.message.c_str());
	}

	try
	{
		control		= 0;
		in			= getcontrol(3);
		nrcontrols	= in[2];

		Util::dlog("DD atmel: found %d pwm outputs\n", nrcontrols);

		min = (in[3] << 8) | (in[4] << 0);
		max = (in[5] << 8) | (in[6] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			Control::capset cc;

			cc.set(Control::cap_isdigital);
			cc.set(Control::cap_canread);
			cc.set(Control::cap_canwrite);
			cc.set(Control::cap_canhardpwm);
			cc.set(Control::cap_canpwmmode);

			control = new Control(root,
					ID(id.interface, id.device, DeviceAtmel::pwm_output, ix + 1),
					min, max, "", 0, cc, DeviceAtmel::pwm_output, ix,
					"pwm", "PWM output");

			if(control)
				controls.add(control);
		}

	}
	catch(minor_exception e)
	{
		Util::dlog("DD atmel: add pwm output: %s\n", e.message.c_str());
	}
}

Util::byte_array DeviceAtmel::command(string cmd, int timeout, int chunks) throw(exception)
{
	Util::byte_array	bytes;
	int					length;
	int					ix;
	uint8_t				checksum;

	bytes	= DeviceI2C::command(cmd + " p r 00", timeout, chunks);
	length	= bytes.size();

	if(length < 3)
		throw(minor_exception("atmel: received too little bytes"));

	checksum = 0;
	for(ix = 0; ix < length - 1; ix++)
		checksum += bytes[ix];

	if(checksum != bytes[length - 1])
		throw(minor_exception("atmel: checksum error"));

	if(bytes[0] != 0x00)
		throw(minor_exception("atmel: device returns error code"));

	return(bytes);
}

Util::byte_array DeviceAtmel::getcontrol(int control) throw(exception)
{
	return(query(0x07, 0, 8, control));; 
}

Util::byte_array DeviceAtmel::query(int cmd, int index, int length, int param1, int param2, int param3, int param4) throw(exception)
{
	ostringstream		in;
	Util::byte_array	bytes;

	cmd = cmd | (index & 0x0f);

	in << "w " << hex << setw(2) << setfill('0') << cmd;

	if(param1 != -1)
		in << " " << hex << setw(2) << setfill('0') << param1;

	if(param2 != -1)
		in << " " << hex << setw(2) << setfill('0') << param2;

	if(param3 != -1)
		in << " " << hex << setw(2) << setfill('0') << param3;

	if(param4 != -1)
		in << " " << hex << setw(2) << setfill('0') << param4;

	try
	{
		bytes = command(in.str());
	}
	catch(minor_exception e)
	{
		Util::dlog("minor exception: %s\n", e.message.c_str());
		throw(minor_exception(string("query: ") + e.message));
	}

	if(bytes.size() != (size_t)length)
		throw(minor_exception("query: invalid result length"));

	return(bytes);
}

