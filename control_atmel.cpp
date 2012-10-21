#include "control_atmel.h"
#include "controls.h"
#include "device.h"
#include "interface.h"
#include "cppstreams.h"
#include "syslog.h"

ControlAtmel::ControlAtmel(Controls *parent_controls,
			int generation_in, int parent_id_in, int ordinal_in, string parent_path_in,
			double min_in, double max_in, string unit_in, int precision_in,
			control_t control_type_in, int index_in) throw(exception)
	:
		Control(parent_controls,
				generation_in, parent_id_in, ordinal_in, parent_path_in,
				min_in, max_in, unit_in, precision_in),
			_control_type(control_type_in), _index(index_in)
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
			throw(minor_exception("ControlAtmel::ControlAtmel: invalid control type"));
		}
	}

	conv.str("");
	conv << "atmel:" << cid << ":" << _index;
	_set_shortname(conv.str());

	conv.str("");
	conv << "Atmel attiny control #" << _ordinal << ": " << cdesc << " " << _index;
	_set_longname(conv.str());
}

ControlAtmel::~ControlAtmel() throw()
{
}

Interface::byte_array ControlAtmel::_query(int cmd, int length, int param1, int param2, int param3, int param4) throw(exception)
{
	ostringstream			in;
	Interface::byte_array	bytes;

	dlog("cmd1: %x\n", cmd);

	if((cmd & 0x0f) == 0x00)
		cmd |= _index;

	dlog("cmd2: %x\n", cmd);

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
		bytes = _controls->device()->command(in.str());
	}
	catch(minor_exception e)
	{
		dlog("minor exception: %s\n", e.message.c_str());
		throw(minor_exception(string("ControlAtmel::_query: ") + e.message));
	}

	dlog("length required: %d\n", length);
	dlog("length actual: %d\n", bytes.size());

	if(bytes.size() != (size_t)length)
		throw(minor_exception("ControlAtmel::_query: invalid result length"));

	return(bytes);
}

double ControlAtmel::read() throw(exception)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			dlog("_query digital input\n");
			bytes = _query(0x30, 4);
			return(double(bytes[2]));
		}

		case(analog_input):
		{
			dlog("_query analog input\n");
			bytes = _query(0xc0, 3);
			bytes = _query(0x01, 5);
			return(double((bytes[2] << 8) | (bytes[3])));
		}

		case(digital_output):
		{
			dlog("_query digital output\n");
			bytes = _query(0x50, 4);
			return(double(bytes[2]));
		}

		case(pwm_output):
		{
			dlog("_query pwm output\n");
			bytes = _query(0x90, 5);
			return(double((bytes[2] << 8) | (bytes[3])));
		}

		default:
		{
			throw(minor_exception("ControlAtmel::read: invalid operation for control"));
		}
	}

	return(-1);
}

void ControlAtmel::write(double value_in) throw(exception)
{
	int value = int(value_in);

	if((value < _min) || (value > _max))
		throw(minor_exception("write control: value out of range"));

	switch(_control_type)
	{
		case(digital_output):
		{
			dlog("_query write digital output\n");
			_query(0x40, 4, value & 0xff);
			break;
		}

		case(pwm_output):
		{
			dlog("_query write pwm output\n");
			_query(0x80, 3, (value & 0xff00) >> 8, value & 0x00ff);
			break;
		}

		default:
		{
			throw(minor_exception("ControlAtmel::write: invalid operation for control"));
		}
	}
}

int ControlAtmel::readcounter() throw(exception)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			dlog("_query: readcounter\n");
			bytes = _query(0x10, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(minor_exception("ControlAtmel::readcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int ControlAtmel::readresetcounter() throw(exception)
{
	Interface::byte_array bytes;

	switch(_control_type)
	{
		case(digital_input):
		{
			dlog("_query: readresetcounter %d\n", _index);
			bytes = _query(0x20, 7);
			return( ((bytes[2] & 0xff000000) >> 24) |
					((bytes[3] & 0x00ff0000) >> 16) |
					((bytes[4] & 0x0000ff00) >>  8) |
					((bytes[5] & 0x000000ff) >>  0));
		}

		default:
		{
			throw(minor_exception("ControlAtmel::readresetcounter: invalid operation for control"));
		}
	}
	
	return(-1);
}

int ControlAtmel::readpwmmode() throw(exception)
{
	Interface::byte_array	bytes;
	int						mode;

	switch(_control_type)
	{
		case(digital_output):
		{
			bytes = _query(0x70, 4);
			mode = bytes[2];

			if(mode == 4)
				mode = 3;

			return(mode);
		}

		case(pwm_output):
		{
			bytes = _query(0xb0, 4);
			mode = bytes[2];

			if(mode == 4)
				mode = 3;

			return(mode);
		}

		default:
		{
			throw(string("ControlAtmel::readpwmmode: invalid operation for control"));
		}
	}

	return(-1);
}

void ControlAtmel::writepwmmode(int value) throw(exception)
{
	if((value < 0) || (value > 3))
		throw(minor_exception("write control: pwm mode out of range"));

	switch(_control_type)
	{
		case(digital_output):
		{
			_query(0x60, 4, value & 0xff);
			break;
		}

		case(pwm_output):
		{
			_query(0xa0, 4, value & 0xff);
			break;
		}

		default:
		{
			throw(minor_exception("ControlAtmel::writepwmmode: invalid operation for control"));
		}
	}
}

string ControlAtmel::readpwmmode_string() throw(exception)
{
	int mode = readpwmmode();

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
