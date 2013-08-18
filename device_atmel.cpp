#include "device_atmel.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"
#include "interface.h"

DeviceAtmel::DeviceAtmel(Interfaces *root_in, ID id_in, void *pdata_in) throw(exception)
	: Device(root_in, id_in, pdata_in)
{
	if(!probe())
		throw(minor_exception(string("no atmel device found at ") + parent()->device_interface_desc(pdata)));
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
	return(string("Atmel AVR"));
}

string DeviceAtmel::name_short() throw()
{
	ostringstream rv;
	rv << name_short_static() << ":" << parent()->device_interface_desc(pdata);
    return(rv.str());
}

string DeviceAtmel::name_long() throw()
{
	ostringstream rv;
	rv << name_long_static() << " (bus: " << parent()->device_interface_desc(pdata) << ")";
    return(rv.str());
}

string DeviceAtmel::device_id() throw()
{
	return(name_short());
}

double DeviceAtmel::read(Control *control) throw(exception)
{
	ByteArray bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = command(1, 0x30 | control->index);
			return(double(bytes[0]));
		}

		case(DeviceAtmel::analog_input):
		{
			bytes = command(2, 0xc0 | control->index);
			return(double((bytes[0] << 8) | (bytes[1])));
		}

		case(DeviceAtmel::digital_output):
		{
			bytes = command(1, 0x50 | control->index);
			return(double(bytes[0]));
		}

		case(DeviceAtmel::pwm_output):
		{
			bytes = command(2, 0x90 | control->index);
			return(double((bytes[0] << 8) | (bytes[1])));
		}

		case(DeviceAtmel::temp_sensor):
		{
			bytes = command(2, 0xd0 | control->index);
			return(double((bytes[0] << 8) | (bytes[1])) / 10.0);
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
			command(0, 0x40 | control->index, value);
			break;
		}

		case(DeviceAtmel::pwm_output):
		{
			command(0, 0x80 | control->index, (value & 0xff00) >> 8, (value & 0x00ff) >> 0);
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
	ByteArray bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = command(4, 0x10 | control->index);
			return( ((bytes[0] & 0xff000000) >> 24) |
					((bytes[1] & 0x00ff0000) >> 16) |
					((bytes[2] & 0x0000ff00) >>  8) |
					((bytes[3] & 0x000000ff) >>  0));
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
	ByteArray bytes;

	switch(control->type)
	{
		case(DeviceAtmel::digital_input):
		{
			bytes = command(4, 0x20 | control->index);
			return( ((bytes[0] & 0xff000000) >> 24) |
					((bytes[1] & 0x00ff0000) >> 16) |
					((bytes[2] & 0x0000ff00) >>  8) |
					((bytes[3] & 0x000000ff) >>  0));
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
	ByteArray	bytes;
	int			mode;

	switch(control->type)
	{
		case(DeviceAtmel::digital_output):
		{
			bytes = command(1, 0x70 | control->index);
			mode = bytes[0];

			if(mode == 4)
				mode = 3;

			return(mode);
		}

		case(DeviceAtmel::pwm_output):
		{
			bytes = command(1, 0xb0 | control->index);
			mode = bytes[0];

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
			command(0, 0x60 | control->index, value);
			break;
		}

		case(DeviceAtmel::pwm_output):
		{
			command(0, 0xa0 | control->index, value);
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
	stringstream	conv;
	ByteArray		in;
	int				ix;

	try
	{
		// 0x00: request identification
		in = command(21, 0x00);

		if((in[0] != 0x4a) || (in[1] != 0xfb))
			throw(minor_exception("device does not identify as atmel"));

		if((in[2] > 3) && (in[2] < 7))
			model = in[2];
		else
			throw(minor_exception("unknown atmel model"));

		version		= in[3];
		revision	= in[4];

		for(ix = 5; ix < 22; ix++)
		{
			if(in[ix] == 0)
				break;
			modelname += (char)in[ix];
		}
	}
	catch(iocd_exception e)
	{
		Util::vlog("II probe atmel: %s\n", e.message.c_str());
		return(false);
	}

	Util::vlog("II device_atmel: detected %s\n", modelname.c_str());
	return(true);
}

void DeviceAtmel::find_controls() throw(exception)
{
	ByteArray	in;
	int			ix;
	int			min, max;
	int			nrcontrols;
	Control		*control;

	try
	{
		in			= command(5, 0x07, 0x00);
		nrcontrols	= in[0];
		control		= 0;

		Util::vlog("II device_atmel: found %d digital inputs\n", nrcontrols);

		min = 0;
		max = (in[1] << 24) | (in[2] << 16) | (in[3] << 8) | in[4];

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
		Util::vlog("II atmel: add digital input: %s\n", e.message.c_str());
	}

	try
	{
		in			= command(5, 0x07, 0x01);
		nrcontrols	= in[0];
		control		= 0;

		Util::vlog("II device_atmel: found %d analog inputs\n", nrcontrols);

		min = (in[1] << 8) | (in[2] << 0);
		max = (in[3] << 8) | (in[4] << 0);

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
		Util::vlog("II atmel: analog input: %s\n", e.message.c_str());
	}

	try
	{
		in			= command(5, 0x07, 0x02);
		nrcontrols	= in[0];
		control		= 0;

		Util::vlog("II device_atmel: found %d digital outputs\n", nrcontrols);

		min = (in[1] << 8) | (in[2] << 0);
		max = (in[3] << 8) | (in[4] << 0);

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
		Util::vlog("II atmel: add digital output: %s\n", e.message.c_str());
	}

	try
	{
		in			= command(5, 0x07, 0x03);
		nrcontrols	= in[0];
		control		= 0;

		Util::vlog("II device_atmel: found %d pwm outputs\n", nrcontrols);

		min = (in[1] << 8) | (in[2] << 0);
		max = (in[3] << 8) | (in[4] << 0);

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
		Util::vlog("II atmel: add pwm output: %s\n", e.message.c_str());
	}

	try
	{
		in			= command(5, 0x07, 0x04);
		nrcontrols	= in[0];
		control		= 0;

		Util::vlog("II device_atmel: found %d temperature sensors\n", nrcontrols);

		min = (in[1] << 8) | (in[2] << 0);
		max = (in[3] << 8) | (in[4] << 0);

		for(ix = 0; ix < nrcontrols; ix++)
		{
			Control::capset cc;

			cc.set(Control::cap_canread);

			control = new Control(root,
					ID(id.interface, id.device, DeviceAtmel::temp_sensor, ix + 1),
					min, max, "˙C", 1, cc, DeviceAtmel::temp_sensor, ix,
					"temp", "Temperature sensor");

			if(control)
				controls.add(control);
		}
	}
	catch(minor_exception e)
	{
		Util::vlog("II atmel: add pwm output: %s\n", e.message.c_str());
	}
}

ByteArray DeviceAtmel::command(int expected_length, int a, int b, int c) throw(exception)
{
	ByteArray in;

	if(a >= 0)
		in.push_back((uint8_t)a);

	if(b >= 0)
		in.push_back((uint8_t)b);

	if(c >= 0)
		in.push_back((uint8_t)c);

	return(command(expected_length, in));
}

ByteArray DeviceAtmel::command(int expected_length, ByteArray in) throw(exception)
{
	ByteArray	out;
	uint8_t		checksum;
	int			ix;
	size_t		length;

	Util::dlog("DD DeviceAtmel::command: send to device: %s\n", string(in).c_str());

	if(write_data(in, 1000) != (ssize_t)in.size())
		throw(major_exception(string("DeviceAtmel::command: failed to write to device")));

	length = read_data(out, 1000);

	if(length < 4)
		throw(major_exception(string("DeviceAtmel::command: reply < 4 bytes: ") + Util::int_to_string(length)));

	if((out[0] + 1) != (uint8_t)length)
		throw(major_exception(string("DeviceAtmel::command: invalid reply length, expected: ") +
					Util::int_to_string(expected_length) + ", received: " + Util::int_to_string(out[0] + 1)));

	if((expected_length > 0) && ((size_t)(expected_length) != (length - 4)))
		throw(major_exception(string("DeviceAtmel::command: unexpected reply length, expected: ") +
					Util::int_to_string(expected_length) + ", received: " + Util::int_to_string(length - 4)));

	if(out[1] != 0x00)
		throw(major_exception(string("DeviceAtmel::command: error from device:") + Util::int_to_string(out[1])));

	if(out[2] != in[0])
		throw(major_exception(string("DeviceAtmel::command: reply on invalid command: ") + Util::int_to_string(in[0]) + "/" + Util::int_to_string(in[2])));

	checksum = 0;

	for(ix = 1; ix < ((int)length - 1); ix++)
		checksum = (checksum + out[ix]) & 0xff;

	if(out[length - 1] != checksum)
		throw(major_exception(string("DeviceAtmel::command: invalid checksum: ") + Util::int_to_string(out[length - 1]) + "/" + Util::int_to_string(checksum)));

	out.erase(out.begin());	//	length
	out.erase(out.begin());	//	error code
	out.erase(out.begin());	//	related command
	out.pop_back();			//	checksum;

	return(out);
}
