#include "control_tsl2550.h"
#include "controls.h"
#include "device.h"
#include "interface.h"
#include "cppstreams.h"
#include "syslog.h"
#include "exception.h"
#include "util.h"

#include <unistd.h>
#include <math.h>

ControlTSL2550::ControlTSL2550(Controls *parent_controls, const Identity &id_in,
			double min_in, double max_in, string unit_in, int precision_in) throw(exception)
	:
		Control(parent_controls, id_in, min_in, max_in, unit_in, precision_in)
{
	_set_shortname("tsl2550:light");
	_set_longname("TSL2550 light sensor");
	_properties = control_props_t(cp_canread);
}

ControlTSL2550::~ControlTSL2550() throw()
{
}

double ControlTSL2550::read() throw(exception)
{
	double lux;

	lux = _read_retry(3, true);

	if(lux < 10)
	{
		dlog("tsl2550::read: reread using standard range, lux = %f\n", lux);
		lux = _read_retry(3, false);
	}

	return(lux);
}

double ControlTSL2550::_read_retry(int attempts, bool erange) throw(exception)
{
	int			attempt;
	exception	exc;

	for(attempt = 0; attempt < attempts; attempt++)
	{
		try
		{
			return(_read_range(erange));
		}
		catch(minor_exception e)
		{
			exc = e;
		}
	}

	throw(exc);
}

double ControlTSL2550::_read_range(bool erange) throw(exception)
{
	Util::byte_array	out;
	string				range_cmd;
	int					range_sleep;
	int					range_multiplier;
	int					ch0, ch1, cch0, cch1;
	bool				overflow;
	double				lux;

	if(erange)
	{
		range_cmd           = "1d";
		range_sleep         = 160000;
		range_multiplier    = 5;
	}
	else
	{
		range_cmd           = "18";
		range_sleep         = 800000;
		range_multiplier    = 1;
	}

	// s <72> p w 00 p w 03 p r 01 p // cycle power ensure fresh readings

	out = _command("w 00 p w 03 p r 01");

	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	if(out[0] != 0x03)
		throw(minor_exception("tsl2550: invalid reply from control\n"));

	// w <18/1d> p r 01 p // select range mode

	out = _command("w " + range_cmd + " p r 01");
	
	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	if(out[0] != 0x1b)
		throw(minor_exception("tsl2550: invalid reply from control\n"));

	usleep(range_sleep);

	// w 43 p r 01 p // select channel 0

	out = _command("w 43 p r 01");
	
	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	ch0 = out[0];

	// w 83 p r 01 p // select channel 1

	out = _command("w 83 p r 01");
	
	if(out.size() != 1)
		throw(minor_exception("tsl2550: invalid reply size from control\n"));

	ch1 = out[0];

	overflow = false;

	if(!_adc2count(ch0, cch0, overflow))
	{
		vlog("tsl2550::read: ch0 invalid\n");
		return(false);
	}

	if(overflow)
	{
		lux = -1;
		return(true);
	}

	if(!_adc2count(ch1, cch1, overflow))
	{
		vlog("tsl2550::read: ch1 invalid\n");
		return(false);
	}

	dlog("ch0 = 0x%x/%d, ch1 = 0x%x/%d\n", ch0, ch0, ch1, ch1);
	dlog("cch0 = %d, cch1 = %d\n", cch0, cch1);

	if(overflow)
	{
		lux = -1;
		return(true);
	}

	lux = _count2lux(cch0, cch1, range_multiplier);
	dlog("lux = %f\n", lux);

	return(lux);
}

bool ControlTSL2550::_adc2count(int in, int &out, bool &overflow)
{
    bool    valid   = !!(in & 0x80);
    int     chord   = (in & 0x70) >> 4;
    int     step    = (in & 0x0f);

    if(!valid)
    {
        //dlog("tsl2550::adc2count: invalid value\n");
        return(false);
    }

    if((in & 0x7f) == 0x7f)
        overflow = true;

    int chordval    = 16.5 * ((1 << chord) - 1);
    int stepval     = step * (1 << chord);

    out = chordval + stepval;

    //dlog("tsl2550::adc2count: valid = %d, chord = %d, step = %d, chordval = %d, stepval = %d, count = %d, overflow = %d\n",
            //valid, chord, step, chordval, stepval, out, (int)overflow);

    return(true);
}

double ControlTSL2550::_count2lux(int ch0, int ch1, int multiplier)
{
    double r, e, l;

    if(ch0 == ch1)
        r = 0;
    else
        r = (double)ch1 / ((double)ch0 - (double)ch1);

    e = exp(-0.181 * r * r);
    l = ((double)ch0 - (double)ch1) * 0.39 * e * (double)multiplier;

    dlog("tsl2550::_count2lux: ch0=%d, ch1=%d, multiplier=%d\n", ch0, ch1, multiplier);
    dlog("tsl2550::_count2lux: r=%f, e=%f, l=%f\n", r, e, l);

    if(l > 100)
        l = round(l);
    else if(l > 10)
        l = round(l * 10) / 10;
    else
        l = round(l * 100)  / 100;

    return(l);
}
