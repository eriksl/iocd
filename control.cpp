#include "interface.h"
#include "device.h"
#include "control.h"
#include "controls.h"
#include "cppstreams.h"
#include "syslog.h"
#include "util.h"

Control::Control(Controls *parent_controls, const Identity &id_in,
				double min_in, double max_in, string unit_in, int precision_in) throw(exception)
	:	Identity(id_in),
			_controls(parent_controls),
			_min(min_in), _max(max_in), _unit(unit_in), _precision(precision_in)
{
}

Control::~Control() throw()
{
}

double Control::min() const throw()
{
	return(_min);
}

string Control::min_string() const throw()
{
	return(_float_to_string(_min, _precision));
}

double Control::max() const throw()
{
	return(_max);
}

string Control::max_string() const throw()
{
	return(_float_to_string(_max, _precision));
}

string Control::unit() const throw()
{
	return(_unit);
}

int Control::precision() const throw()
{
	return(_precision);
}

string Control::precision_string() const throw()
{
	return(_int_to_string(_precision));
}

string Control::properties() const throw()
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

	if(canpwm())
	{
		if(!result.empty())
			result += "/";
		result += "pwm";
	}

	return(result);
}

bool Control::canread() const throw()
{
	return(!!(_properties & cp_canread));
}

bool Control::canwrite() const throw()
{
	return(!!(_properties & cp_canwrite));
}

bool Control::cancount() const throw()
{
	return(!!(_properties & cp_cancount));
}

bool Control::canpwm() const throw()
{
	return(!!(_properties & cp_canpwm));
}

bool Control::isdigital() const throw()
{
	return(!!(_properties & cp_isdigital));
}

Controls* Control::controls() throw()
{
	return(_controls);
}

Device* Control::device() throw()
{
	return(controls()->device());
}

Devices* Control::devices() throw()
{
	return(device()->devices());
}

Interface* Control::interface() throw()
{
	return(devices()->interface());
}

double Control::read() throw(exception)
{
	throw(minor_exception("control does not implement read"));
}

void Control::write(double) throw(exception)
{
	throw(minor_exception("control does not implement write"));
}

double Control::readwrite(double newvalue) throw(exception)
{
	double oldvalue = this->read();
	this->write(newvalue);
	return(oldvalue);
}

int Control::readcounter() throw(exception)
{
	throw(minor_exception("control does not implement readcounter"));
}

int Control::readresetcounter() throw(exception)
{
	throw(minor_exception("control does not implement readresetcounter"));
}

int Control::readpwmmode() throw(exception)
{
	throw(minor_exception("control does not implement readpwmmode"));
}

void Control::writepwmmode(int) throw(exception)
{
	throw(minor_exception("control does not implement writepwmmode"));
}

string Control::read_string() throw(exception)
{
	return(_float_to_string(read(), _precision));
}

string Control::readwrite_string(double value) throw(exception)
{
	return(_float_to_string(readwrite(value), _precision));
}

string Control::readcounter_string() throw(exception)
{
	return(_int_to_string(readcounter()));
}

string Control::readresetcounter_string() throw(exception)
{
	return(_int_to_string(readresetcounter()));
}

string Control::readpwmmode_string() throw(exception)
{
	return(_int_to_string(readpwmmode()));
}

Util::byte_array Control::_command(string cmd, int timeout, int chunks) throw(exception)
{
	return(_controls->device()->command(cmd, timeout, chunks));
}

string Control::_int_to_string(int in) throw()
{
	stringstream conv;
	conv << in;
	return(conv.str());
}

string Control::_float_to_string(double value, int precision) throw()
{
	stringstream conv;
	conv << fixed << setprecision(precision) << value;
	return(conv.str());
}
