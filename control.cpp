#include "interfaces.h"
#include "control.h"
#include "device.h"
#include "cppstreams.h"
#include "util.h"

Control::Control(Interfaces *root_in, ID id_in,
				double min_in, double max_in, string unit_in, int precision_in,
				const Control::capset &caps_in, int type_in, int index_in,
				string name_short_in, string name_long_in) throw(exception)
	:		id(id_in),
			min(min_in),
			max(max_in),
			unit(unit_in),
			precision(precision_in),
			type(type_in),
			caps(caps_in),
			index(index_in),
			name_short(name_short_in),
			name_long(name_long_in),
			control_id(name_short_in + ":" + Util::int_to_string(index_in)),
			root(root_in)
{
}

Control::~Control() throw()
{
}

string Control::min_string() const throw()
{
	return(Util::float_to_string(min, precision));
}

string Control::max_string() const throw()
{
	return(Util::float_to_string(max, precision));
}

string Control::precision_string() const throw()
{
	return(Util::int_to_string(precision));
}

string Control::type_string() const throw()
{
	return(Util::int_to_string(type));
}

string Control::index_string() const throw()
{
	return(Util::int_to_string(index));
}

string Control::capabilities() const throw()
{
	string result;

	if(isdigital())
		result = "digital";
	else
		result = "analog";

	if(canread())
	{
		if(!result.empty())
			result += " ";
		result += "input";
	}

	if(canwrite())
	{
		if(!result.empty())
			result += " ";
		result += "output";
	}

	if(cancount())
	{
		if(!result.empty())
			result += "/";
		result += "counter";
	}

	if(cansoftpwm())
	{
		if(!result.empty())
			result += "/";
		result += "softpwm";
	}

	if(canhardpwm())
	{
		if(!result.empty())
			result += "/";
		result += "hardpwm";
	}

	if(canpwmmode())
	{
		if(!result.empty())
			result += "/";
		result += "pwmmode";
	}

	return(result);
}

bool Control::isdigital() const throw()
{
	return(caps.test(Control::cap_isdigital));
}

bool Control::canread() const throw()
{
	return(caps.test(Control::cap_canread));
}

bool Control::canwrite() const throw()
{
	return(caps.test(Control::cap_canwrite));
}

bool Control::cancount() const throw()
{
	return(caps.test(Control::cap_cancount));
}

bool Control::cansoftpwm() const throw()
{
	return(caps.test(Control::cap_cansoftpwm));
}

bool Control::canhardpwm() const throw()
{
	return(caps.test(Control::cap_canhardpwm));
}

bool Control::canpwm() const throw()
{
	return(caps.test(Control::cap_canhardpwm) || caps.test(Control::cap_cansoftpwm));
}

bool Control::canpwmmode() const throw()
{
	return(caps.test(Control::cap_canpwmmode));
}

double Control::read() throw(exception)
{
	return(parent()->read(this));
}

void Control::write(double value) throw(exception)
{
	return(parent()->write(this, value));
}

double Control::readwrite(double value) throw(exception)
{
	return(parent()->readwrite(this, value));
}

int Control::readcounter() throw(exception)
{
	return(parent()->readcounter(this));
}

int Control::readresetcounter() throw(exception)
{
	return(parent()->readresetcounter(this));
}

int Control::readpwmmode() throw(exception)
{
	return(parent()->readpwmmode(this));
}

void Control::writepwmmode(int value) throw(exception)
{
	return(parent()->writepwmmode(this, value));
}

string Control::read_string() throw(exception)
{
	return(Util::float_to_string(read(), precision));
}

string Control::readwrite_string(double value) throw(exception)
{
	return(Util::float_to_string(readwrite(value), precision));
}

string Control::readcounter_string() throw(exception)
{
	return(Util::int_to_string(readcounter()));
}

string Control::readresetcounter_string() throw(exception)
{
	return(Util::int_to_string(readresetcounter()));
}

string Control::readpwmmode_string() throw(exception)
{
	return(Util::int_to_string(readpwmmode()));
}

Util::byte_array Control::command(string cmd, int timeout, int chunks) throw(exception)
{
	return(parent()->command(cmd, timeout, chunks));
}

Device* Control::parent() throw(exception)
{
	return(root->find_device(id));
}
