#include "control_digipicco.h"
#include "controls.h"
#include "device.h"
#include "interface.h"
#include "cppstreams.h"
#include "syslog.h"
#include "exception.h"

#include <unistd.h>

ControlDigipicco::ControlDigipicco(Controls *parent_controls,
			int generation_in, int parent_id_in, int ordinal_in, string parent_path_in,
			double min_in, double max_in, string unit_in, int precision_in,
			ControlDigipicco::control_t type) throw(exception)
	:
		Control(parent_controls,
				generation_in, parent_id_in, ordinal_in, parent_path_in,
				min_in, max_in, unit_in, precision_in),
		_type(type)
{
	switch(type)
	{
		case(ControlDigipicco::control_temperature):
		{
			_set_shortname("digipicco:temp");
			_set_longname("digipicco temperature reading");
			break;
		}

		case(ControlDigipicco::control_humidity):
		{
			_set_shortname("digipicco:hum");
			_set_longname("digipicco humidity reading");
			break;
		}

		default:
		{
			throw(major_exception("invalid control type for ControlDigipicco"));
		}
	}

	_properties = control_props_t(cp_canread);
}

ControlDigipicco::~ControlDigipicco() throw()
{
}

double ControlDigipicco::read() throw(exception)
{
	Interface::byte_array	bytes;
	uint16_t				rv[2];
	int						attempt;
	double					temperature, humidity;

	try
	{
		for(attempt = 0; attempt < 3; attempt++)
		{
			bytes = _controls->device()->command("r 04 ");

			if(bytes.size() == 4)
				break;

			usleep(100000);
		}

		if(attempt > 2)
			throw(minor_exception("incorrect length in reply"));

		rv[0] = (bytes[0] << 8) | (bytes[1] << 0);
		rv[1] = (bytes[2] << 8) | (bytes[3] << 0);

		if((rv[0] == 0xffff) && (rv[1] == 0xffff))
			throw(minor_exception("incorrect reply"));
	}
	catch(minor_exception error)
	{
		throw(minor_exception(string("read digipicco: ") + error.message));
	}

	temperature = (((double)rv[1] / 32767) * 165) - 40;
	humidity	= (double)rv[0] / 327.67;

	return(_type == ControlDigipicco::control_temperature ? temperature : humidity);
}
