#include "control_atmel.h"
#include "interface.h"
#include "device.h"
#include "cppstreams.h"
#include "syslog.h"

ControlAtmel::ControlAtmel(Interface *parent_interface, Device *parent_device, 
		int min_in, int max_in, string unit_in, control_t control_type_in, int ordinal_in) throw(string) :
	Control(parent_interface, parent_device, min_in, max_in, unit_in),
	_control_type(control_type_in), _ordinal(ordinal_in)
{
	stringstream	conv;
	string			cdesc;
	string			cid;

	switch(control_type_in)
	{
		case(digital_input):
		{
			cid		= "din";
			cdesc	= "digital input";
			_properties = control_props_t(cp_canread | cp_cancount | cp_isdigital);
			break;
		}

		case(analog_input):
		{
			cid		= "ain";
			cdesc	= "analog input";
			_properties = cp_canread;
			break;
		}

		case(digital_output):
		{
			cid		= "dout";
			cdesc	= "digital output";
			_properties = control_props_t(cp_canread | cp_canwrite | cp_canpwm | cp_isdigital);
			break;
		}

		case(pwm_output):
		{
			cid		= "pwm";
			cdesc	= "pwm output";
			_properties = control_props_t(cp_canread | cp_canwrite | cp_canpwm | cp_isdigital);
			break;
		}

		default:
		{
			throw(string("ControlAtmel::ControlAtmel: invalid control type"));
		}
	}

	conv << cdesc << " " << ordinal_in;
	_name = conv.str();

	conv.str("");
	conv << "atmel:" << cid << "_" << ordinal_in;
	_bus = conv.str();
}

ControlAtmel::~ControlAtmel() throw()
{
}

Interface::byte_array ControlAtmel::_query(int cmd, int length, int param1, int param2, int param3, int param4) throw(string)
{
	stringstream			in;
	Interface::byte_array	bytes;

	if(!(cmd & 0x0f))
		cmd |= _ordinal;

	in << "w " << hex << setw(2) << setfill('0') << cmd;

	if(param1 != -1)
		in << param1 << " ";

	if(param2 != -1)
		in << param2 << " ";

	if(param3 != -1)
		in << param3 << " ";

	if(param4 != -1)
		in << param4 << " ";

	bytes = _device->command(in.str());

	if(bytes.size() != (size_t)length)
		throw(string("ControlAtmel::_query: invalid result length"));

	return(bytes);
}

int ControlAtmel::read() throw(string)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			bytes = _query(0x30, 4);
			return(bytes[2]);
		}

		case(analog_input):
		{
			bytes = _query(0xc0, 3);
			bytes = _query(0x01, 5);
			return((bytes[2] << 8) | (bytes[3]));
		}

		case(digital_output):
		{
			bytes = _query(0x50, 4);
			return(bytes[2]);
		}

		case(pwm_output):
		{
			bytes = _query(0x90, 5);
			return((bytes[2] << 8) | (bytes[3]));
		}

		default:
		{
			throw(string("ControlAtmel::read: invalid operation for control"));
		}
	}

	return(-1);
}

void ControlAtmel::write(int value) throw(string)
{
	switch(_control_type)
	{
		case(digital_output):
		{
			_query(0x40, 3, value & 0xff);
			break;
		}

		case(pwm_output):
		{
			_query(0x80, 3, (value & 0xff00) >> 8, value & 0x00ff);
			break;
		}

		default:
		{
			throw(string("ControlAtmel::write: invalid operation for control"));
		}
	}
}

int ControlAtmel::readcounter() throw(string)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			bytes = _query(0x10, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(string("ControlAtmel::readcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int ControlAtmel::readresetcounter() throw(string)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			bytes = _query(0x20, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(string("ControlAtmel::readresetcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int ControlAtmel::readpwmmode() throw(string)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_output):
		{
			bytes = _query(0x70, 4);
			return(bytes[2]);
		}

		case(pwm_output):
		{
			bytes = _query(0xb0, 4);
			return(bytes[2]);
		}

		default:
		{
			throw(string("ControlAtmel::readpwmmode: invalid operation for control"));
		}
	}

	return(-1);
}

void ControlAtmel::writepwmmode(int value) throw(string)
{
	switch(_control_type)
	{
		case(digital_output):
		{
			_query(0x60, 3, value & 0xff);
			break;
		}

		case(pwm_output):
		{
			_query(0xa0, 3, value & 0xff);
			break;
		}

		default:
		{
			throw(string("ControlAtmel::writepwmmode: invalid operation for control"));
		}
	}
}
