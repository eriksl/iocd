#include "control.h"
#include "interface.h"
#include "device.h"

Control::Control(Interface *parent_interface, Device *parent_device,
		int min_in, int max_in, string unit_in) throw(string)
		: _interface(parent_interface), _device(parent_device),
		_min(min_in), _max(max_in), _unit(unit_in)
{
}

Control::~Control() throw()
{
}

Interface* Control::interface() const throw()
{
	return(_interface);
}

Device* Control::device() const throw()
{
	return(_device);
}

string Control::id() const throw()
{
	return(_interface->bus() + "@" + _device->bus() + "@" + _bus);
}

string Control::name() const throw()
{
	return(_name);
}
string Control::bus() const throw()
{
	return(_bus);
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

int Control::min() const throw()
{
	return(_min);
}

int Control::max() const throw()
{
	return(_max);
}

string Control::unit() const throw()
{
	return(_unit);
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

int Control::read() throw(string)
{
	throw(string("control does not implement read"));
}

void Control::write(int) throw(string)
{
	throw(string("control does not implement write"));
}

int Control::readwrite(int newvalue) throw(string)
{
	int oldvalue = this->read();
	this->write(newvalue);
	return(oldvalue);
}

int Control::readcounter() throw(string)
{
	throw(string("control does not implement readcounter"));
}

int Control::readresetcounter() throw(string)
{
	throw(string("control does not implement readresetcounter"));
}

int Control::readpwmmode() throw(string)
{
	throw(string("control does not implement readpwmmode"));
}

void Control::writepwmmode(int) throw(string)
{
	throw(string("control does not implement writepwmmode"));
}
