#include "interface.h"
#include "device_ds1731.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"
#include "if_private_data.h"

DeviceDS1731::DeviceDS1731(Interfaces *root_in, ID id_in, const InterfacePrivateData *pd_in) throw(exception)
	:	Device(root_in, id_in, pd_in)
{
	if(!probe())
		throw(minor_exception(string("ds1731 not detected at ") + parent()->device_interface_desc(*private_data)));
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
	return("DS1731 digital temperature sensor");
}

string DeviceDS1731::name_short() throw()
{
	ostringstream rv;
	rv << name_short_static() << "@" << parent()->device_interface_desc(*private_data);
	return(rv.str());
}

string DeviceDS1731::name_long() throw()
{
	ostringstream rv;
	rv << name_long_static() << " (bus: " << parent()->device_interface_desc(*private_data) << ")";
	return(rv.str());
}

double DeviceDS1731::read(Control *) throw(exception)
{
	int8_t				v0;
	uint8_t				v1;
	ByteArray			bytes;

	try
	{
		// "w aa r 02 p" read temperature

		write_data(1000, 0xaa);

		if(read_data(bytes, 0x02, 1000) != 2)
			throw(minor_exception("invalid reply"));
	}
	catch(iocd_exception error)
	{
		throw(major_exception(string("read ds1731: ") + error.message));
	}

	v0 = bytes[0];
	v1 = bytes[1];

	return(double(((v0 << 8) | v1) >> 4) * 0.0625);
}

bool DeviceDS1731::probe() throw()
{
	ByteArray bytes;

	try
	{
		// "w 54 p" // Software POR (cycle power)
		
		write_data(1000, 0x54);

		// "w 22 p" // Stop Convert T (stop measuring continiously)

		write_data(1000, 0x22);

		// "w ac r 01 p" Access Config (check config register)

		write_data(1000, 0xac);
	
		if(read_data(bytes, 0x01, 1000) != 1)
			throw(minor_exception("incorrect length in reply"));

		if((bytes[0] & 0xfc) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w ac 8c r 01 p" Access Config (put device in 12 bits / continuous mode)
		
		write_data(1000, 0xac, 0x8c);

		if(read_data(bytes, 0x01, 1000) != 1)
			throw(minor_exception("incorrect length in reply"));

		if((bytes[0] & 0xef) != 0x8c)
			throw(minor_exception("incorrect reply from probe"));

		// "w 51 p" Start Convert T (start convert)
		
		write_data(1000, 0x51);
	}
	catch(iocd_exception e)
	{
		Util::dlog("ds1731 not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: ds1731 detected\n");
	return(true);
}

void DeviceDS1731::find_controls() throw(exception)
{
	Control *control = 0;

	try
	{
		Control::capset cp;
		cp.set(Control::cap_canread);

		control = new Control(root, ID(id.interface, id.device, 1, 1),
				-55, 125, "˙C", 2, cp, 0, 0,
				"temp", "temperature");
	}
	catch(minor_exception e)
	{
		Util::dlog("CC ds1731 controls not found: %s\n", e.message.c_str());
	}

	if(control)
		controls.add(control);
}
