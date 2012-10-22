#include "control_ds1731.h"
#include "controls.h"
#include "device.h"
#include "interface.h"
#include "cppstreams.h"
#include "syslog.h"
#include "exception.h"
#include "util.h"

#include <unistd.h>
#include <math.h>

ControlDS1731::ControlDS1731(Controls *parent_controls, const Identity &id_in,
			double min_in, double max_in, string unit_in, int precision_in) throw(exception)
	:
		Control(parent_controls, id_in, min_in, max_in, unit_in, precision_in)
{
	_set_shortname("ds1731:temp");
	_set_longname("DS1731 temperature sensor");
	_properties = control_props_t(cp_canread);
}

ControlDS1731::~ControlDS1731() throw()
{
}

double ControlDS1731::read() throw(exception)
{
	Util::byte_array	out;
	bool				negative;
	int					temp_hex;
	double				temp;

	// "w aa r 02 p" read temperature
	out = _command("w aa r 02 p");

	if(out.size() != 2)
		throw(minor_exception("read_ds1731: invalid reply"));

	negative	= !!(out[0] & 0x80);
	temp_hex	= (((out[0] & 0x7f) << 8) | (out[1] & 0xf0)) >> 4;
	temp		= double(temp_hex) * 0.0625;

	if(negative)
		temp = 0 - temp;

	return(temp);
}
