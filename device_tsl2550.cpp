#include "device_tsl2550.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>
#include <math.h>

DeviceTSL2550::DeviceTSL2550(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:
		DeviceI2C(root_in, id_in, address_in)
{
	if(!probe())
		throw(minor_exception(string("no tsl2550 device found at 0x") + Util::int_to_string(address)));
}

DeviceTSL2550::~DeviceTSL2550() throw()
{
}

string DeviceTSL2550::name_short_static() throw()
{
	return("tsl2550");
}

string DeviceTSL2550::name_long_static() throw()
{
	return("TSL2550 light sensor");
}

string DeviceTSL2550::name_short() const throw()
{
	return(name_short_static());
}

string DeviceTSL2550::name_long() const throw()
{
	return(name_long_static());
}

bool DeviceTSL2550::probe() throw()
{
	ostringstream		control_name_short, control_name_long;
	Util::byte_array	out;

	try
	{
		// s <72> p w 00 p w 03 p r 01 p // cycle power ensure fresh readings

		out = command("w 00 p w 03 p r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if(out[0] != 0x03)
			throw(minor_exception("incorrect reply from probe"));
	}
	catch(minor_exception e)
	{
		Util::dlog("tsl2550 not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: tsl2550 detected\n");

	return(true);
}

void DeviceTSL2550::find_controls() throw()
{
	Control			*control = 0;
	Control::capset cc;

	cc.set(Control::cap_canread);

	try
	{
		control = new Control(root,
				ID(id.interface, id.device, 1, 1),
				0, 6500, "lux", 2, cc, 0, 0, "light", "Light sensor");
	}
	catch(minor_exception e)
	{
		Util::dlog("tsl2550 control: %s\n", e.message.c_str());
	}

	if(control)
		controls.add(control);
}

bool DeviceTSL2550::adc2count(int in, int &out, bool &overflow) throw()
{
    bool    valid   = !!(in & 0x80);
    int     chord   = (in & 0x70) >> 4;
    int     step    = (in & 0x0f);

    if(!valid)
        return(false);

    if((in & 0x7f) == 0x7f)
        overflow = true;

    int chordval    = 16.5 * ((1 << chord) - 1);
    int stepval     = step * (1 << chord);

    out = chordval + stepval;

    return(true);
}

double DeviceTSL2550::count2lux(int ch0, int ch1, int multiplier) throw()
{
    double r, e, l;

    if(ch0 == ch1)
        r = 0;
    else
        r = (double)ch1 / ((double)ch0 - (double)ch1);

    e = exp(-0.181 * r * r);
    l = ((double)ch0 - (double)ch1) * 0.39 * e * (double)multiplier;

    if(l > 100)
        l = round(l);
    else if(l > 10)
        l = round(l * 10) / 10;
    else
        l = round(l * 100)  / 100;

    return(l);
}

double DeviceTSL2550::read_retry(int attempts, sens_t sensitivity) throw(exception)
{
	int			attempt;
	exception	saved_exception;

	for(attempt = 0; attempt < attempts; attempt++)
	{
		try
		{
			return(read_sens(sensitivity));
		}
		catch(minor_exception e)
		{
			saved_exception = e;
		}
	}

	throw(saved_exception);
}

double DeviceTSL2550::read_sens(sens_t sensitivity) throw(exception)
{
	Util::byte_array	out;
	string				sens_cmd;
	int					sens_multiplier;
	int					retry;
	int					ch0, ch1, cch0, cch1;
	bool				overflow;
	double				lux;
	string				sens_id;

	if(sensitivity == sens_low)
	{
		sens_cmd			= "1d";
		sens_multiplier		= 5;
		sens_id				= "low";
	}
	else
	{
		sens_cmd			= "18";
		sens_multiplier		= 1;
		sens_id				= "high";
	}

	// s <72> p w 00 p w 03 p r 01 p // cycle power ensure fresh readings

	out = command("w 00 p w 03 p r 01");

	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	if(out[0] != 0x03)
		throw(minor_exception("tsl2550: invalid reply from control\n"));

	// w <18/1d> p r 01 p // select range mode

	out = command("w " + sens_cmd + " p r 01");
	
	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	if(out[0] != 0x1b)
		throw(minor_exception("tsl2550: invalid reply from control\n"));

	for(retry = 32; retry > 0; retry--)
	{
		usleep(20000);

		// w 43 p r 01 p // select channel 0

		out = command("w 43 p r 01");
	
		if(out.size() != 1)
			throw(minor_exception("tsl2550: invalid reply size from control\n"));

		ch0 = out[0];

		// w 83 p r 01 p // select channel 1

		out = command("w 83 p r 01");
	
		if(out.size() != 1)
			throw(minor_exception("tsl2550: invalid reply size from control\n"));

		ch1 = out[0];

		overflow = false;

		if(!adc2count(ch0, cch0, overflow))
		{
			Util::dlog("tsl2550::read(%s): ch0 invalid, retry: %d\n", sens_id.c_str(), retry);
			continue;
		}

		if(overflow)
		{
			//Util::dlog("ch0 overflow\n");
			return(-1);
		}

		if(!adc2count(ch1, cch1, overflow))
		{
			Util::dlog("tsl2550::read(%s): ch1 invalid, retry: %d\n", sens_id.c_str(), retry);
			continue;
		}

		if(overflow)
		{
			//Util::dlog("ch1 overflow\n");
			return(-1);
		}

		break;
	}

	if(retry == 0)
		throw(minor_exception("tsl2550::read: invalid data retry exceeded"));

	Util::dlog("ch0 = 0x%x/%d, ch1 = 0x%x/%d\n", ch0, ch0, ch1, ch1);
	Util::dlog("cch0 = %d, cch1 = %d\n", cch0, cch1);

	lux = count2lux(cch0, cch1, sens_multiplier);
	Util::dlog("lux(%s) = %f\n", sens_id.c_str(), lux);

	return(lux);
}

double DeviceTSL2550::read(Control *) throw(exception)
{
	double lux;

	lux = read_retry(3, sens_low);

	if(lux == -1)
	{
		Util::dlog("tsl2550::read: overflow\n");
		return(3000);
	}

	if(lux < 10)
		lux = read_retry(3, sens_high);

	return(lux);
}
