#include "device_ds1731.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

DeviceDS1731::DeviceDS1731(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:	DeviceI2C(root_in, id_in, address_in)
{
	if(!probe())
		throw(minor_exception(string("no ds1731 device found at ") + Util::int_to_string(address)));
}

DeviceDS1731::~DeviceDS1731() throw()
{
}

string DeviceDS1731::name_short_static() throw()
{
	return("ds1731");
}

string DeviceDS1731::name_long_static() throw()
{
	return("DS1731 temperature sensor");
}

string DeviceDS1731::name_short() const throw()
{
	return(name_short_static());
}

string DeviceDS1731::name_long() const throw()
{
	return(name_long_static());
}

double DeviceDS1731::read(Control *) throw(exception)
{
	Util::byte_array	out;
	bool				negative;
	int					temp_hex;
	double				temp;

	// "w aa r 02 p" read temperature
	out = command("w aa r 02 p");

	if(out.size() != 2)
		throw(minor_exception("read_ds1731: invalid reply"));

	negative	= !!(out[0] & 0x80);
	temp_hex	= (((out[0] & 0x7f) << 8) | (out[1] & 0xf0)) >> 4;
	temp		= double(temp_hex) * 0.0625;

	if(negative)
		temp = 0 - temp;

	return(temp);
}

bool DeviceDS1731::probe() throw()
{
	Util::byte_array out;

	try
	{
		// "w 54 p" // Software POR (cycle power)

		out = command("w 54 r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		// "w 22 p" // Stop Convert T (stop measuring continiously)

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		out = command("w 22 r 01");

		// "w ac r 01 p" Access Config (check config register)

		out = command("w ac r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if((out[0] & 0xfc) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w ac 8c r 01 p" Access Config (put device in 12 bits / continuous mode)

		out = command("w ac 8c r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if((out[0] & 0xef) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w 51 p" Start Convert T (start convert)

		out = command("w 51 r 01");

		if(out.size() != 1)
			throw(minor_exception("incorrect length in reply"));
	}
	catch(minor_exception e)
	{
		Util::dlog("ds1731 not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: ds1731 detected\n");
	return(true);
}

void DeviceDS1731::find_controls() throw(exception)
{
	Control			*control = 0;

	try
	{
		Control::capset cp;
		cp.set(Control::cap_canread);

		control = new Control(root, ID(id.interface, id.device, 1, 1),
				-55, 125, "˙C", 2, cp, 0, 0,
				"temp", "Temperature sensor");
	}
	catch(minor_exception e)
	{
		Util::dlog("CC ds1731 controls not found: %s\n", e.message.c_str());
	}

	if(control)
		controls.add(control);
}
