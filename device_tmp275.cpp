#include "interface.h"
#include "device_tmp275.h"
#include "control.h"
#include "id.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

DeviceTMP275::DeviceTMP275(Interfaces *root_in, ID id_in, const InterfacePrivateData *pdata_in) throw(exception)
	:	Device(root_in, id_in, pdata_in)
{
	if(!probe())
		throw(minor_exception(string("tmp275 not detected at ") + parent()->device_interface_desc(*private_data)));
}

DeviceTMP275::~DeviceTMP275() throw()
{
}

string DeviceTMP275::name_short_static() throw()
{
	return("tmp275");
}

string DeviceTMP275::name_long_static() throw()
{
	return("TMP275 temperature sensor");
}

string DeviceTMP275::name_short() throw()
{
	ostringstream rv;
	rv << name_short_static() << "@" << parent()->device_interface_desc(*private_data);
	return(rv.str());
}

string DeviceTMP275::name_long() throw()
{
	ostringstream rv;
	rv << name_long_static() << " (bus: " << parent()->device_interface_desc(*private_data) << ")";
	return(rv.str());
}

double DeviceTMP275::read(Control *) throw(exception)
{
	ByteArray	bytes;
	int16_t		tmp;

	try
	{
		write_data(1000, 0x00);
		read_data(bytes, 0x02, 1000);
	}
	catch(minor_exception error)
	{
		throw(minor_exception(string("read tmp275: ") + error.message));
	}

	tmp = ((bytes[0] << 8) | (bytes[1] << 0)) >> 4;

	return(double(tmp) * 0.0625);
}

bool DeviceTMP275::probe() throw()
{
	stringstream	conv;
	ByteArray		in;

	try
	{
		// write 0xabcd in threshold register,
		// check treshold register, it should return 0xabc0
		
		write_data(1000, 0x02, 0xab, 0xcd);
		read_data(in, 0x02, 1000);

		if((in[0] != 0xab) || (in[1] != 0xc0))
			throw(minor_exception("incorrect reply from probe"));

		// write to config register
		// os (oneshot) = 0, r1/r0 (resolution) = 11 = 12 bits,
		// f1/f0 (fault queue) = 00 (1) (n/a)
		// pol (polarity of the ALERT pin) = 0 (low) (n/a)
		// tm (thermostat mode) = 0 (comparator) (n/a)
		// sd (shutdown) = no shutdown
 
		in.clear();
		write_data(100, 0x01, 0x60);
		read_data(in, 0x01, 100);

		if(in[0] != 0x60)
			throw(minor_exception("incorrect reply from probe"));

	}
	catch(minor_exception e)
	{
		Util::dlog("tmp275 not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: tmp275 detected\n");

	return(true);
}

void DeviceTMP275::find_controls() throw()
{
	Control *control = 0;

	try
	{
		Control::capset cc;
		cc.set(Control::cap_canread);

		control = new Control(root, ID(id.interface, id.device, 1, 1),
				-40, 125, "˙C", 2, cc, 0, 0, "temp", "Temperature");
	}
	catch(minor_exception e)
	{
		Util::dlog("CC tmp275: %s\n", e.message.c_str());
	}

	if(control)
		controls.add(control);
}
