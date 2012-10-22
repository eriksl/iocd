#include "control_tmp275.h"
#include "controls.h"
#include "device.h"
#include "interface.h"
#include "cppstreams.h"
#include "syslog.h"
#include "exception.h"
#include "util.h"

#include <unistd.h>

ControlTMP275::ControlTMP275(Controls *parent_controls,
			int generation_in, int parent_id_in, int ordinal_in, string parent_path_in,
			double min_in, double max_in, string unit_in, int precision_in) throw(exception)
	:
		Control(parent_controls,
				generation_in, parent_id_in, ordinal_in, parent_path_in,
				min_in, max_in, unit_in, precision_in)
{
	_set_shortname("tmp275:temp0");
	_set_longname("TMP275 temperature reading");
	_properties = control_props_t(cp_canread);
}

ControlTMP275::~ControlTMP275() throw()
{
}

double ControlTMP275::read() throw(exception)
{
	Util::byte_array	bytes;
	int16_t				tmp;

	try
	{
		// put device in 12 bits / shutdown mode, take one ("one shot") measurement
		bytes = _controls->device()->command("w 01 e1 r 01");

		if(bytes.size() != 1)
			throw(minor_exception("invalid reply size from control"));

		if(bytes[0] != 0x61)
			throw(minor_exception("invalid reply from control"));

		// conversion time takes 300 ms max
		usleep(300000);

		bytes = _controls->device()->command("w 00 r 02");

		if(bytes.size() != 2)
			throw(minor_exception("invalid reply size from control"));
	}
	catch(minor_exception error)
	{
		throw(minor_exception(string("read tmp275: ") + error.message));
	}

	tmp = ((bytes[0] << 8) | (bytes[1] << 0)) >> 4;

	return(double(tmp) * 0.0625);
}
