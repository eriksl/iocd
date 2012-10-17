#include "control.h"
#include "cppstreams.h"

Control::Control(Controls *parent_controls,
				int generation_in, int parent_id_in, int ordinal_in, string parent_path_in,
				int min_in, int max_in, string unit_in) throw(string)
	:	Identity(generation_in, parent_id_in, ordinal_in, parent_path_in),
			_controls(parent_controls), _min(min_in), _max(max_in), _unit(unit_in)
{
}

Control::~Control() throw()
{
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
