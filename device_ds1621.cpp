#include "interface.h"
#include "device_ds1621.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"
#include "if_private_data.h"

DeviceDS1621::DeviceDS1621(Interfaces *root_in, ID id_in, const InterfacePrivateData *pd_in) throw(exception)
	:	Device(root_in, id_in, pd_in)
{
	if(!probe())
		throw(minor_exception(string("ds1621 not detected at ") + parent()->device_interface_desc(*private_data)));
}

DeviceDS1621::~DeviceDS1621() throw()
{
}

string DeviceDS1621::name_short_static() throw()
{
	return("ds1621");
}

string DeviceDS1621::name_long_static() throw()
{
	return("DS1621 9-bits temperature sensor");
}

string DeviceDS1621::name_short() throw()
{
	ostringstream rv;
	rv << name_short_static() << "@" << parent()->device_interface_desc(*private_data);
	return(rv.str());
}

string DeviceDS1621::name_long() throw()
{
	ostringstream rv;
	rv << name_long_static() << " (bus: " << parent()->device_interface_desc(*private_data) << ")";
	return(rv.str());
}

double DeviceDS1621::read(Control *) throw(exception)
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
		throw(major_exception(string("read ds1621: ") + error.message));
	}

	v0 = bytes[0];
	v1 = bytes[1];

	return(double(((v0 << 8) | v1) >> 4) * 0.0625);
}

bool DeviceDS1621::probe() throw()
{
	ByteArray bytes;

	try
	{
		// "w 22 p" // Stop Convert T (stop measuring continiously)

		write_data(1000, 0x22);

		// "w a1 p" // temperature threshold high register

		write_data(1000, 0xa1);

		if(read_data(bytes, 0x02, 1000) != 2)
			throw(minor_exception("incorrect length in reply"));

		if((bytes[0] != 0x4a) || (bytes[1] != 0x00))
			throw(minor_exception("incorrect reply from probe"));

		// "w a2 p" // temperature threshold low register

		write_data(1000, 0xa2);

		if(read_data(bytes, 0x02, 1000) != 2)
			throw(minor_exception("incorrect length in reply"));

		if((bytes[0] != 0xfb) || (bytes[1] != 0x00))
			throw(minor_exception("incorrect reply from probe"));

		// "w ac 00 r 01 p" Access Config (continuous mode)
		
		write_data(1000, 0xac, 0x00);

		if(read_data(bytes, 0x01, 1000) != 1)
			throw(minor_exception("incorrect length in reply"));

		if((bytes[0] & 0x0f) != 0x00)
			throw(minor_exception("incorrect reply from probe"));

		// "w ee p" Start Convert T (start continuous convert)
		
		write_data(1000, 0xee);
	}
	catch(iocd_exception e)
	{
		Util::dlog("ds1621 not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: ds1621 detected\n");
	return(true);
}

void DeviceDS1621::find_controls() throw(exception)
{
	Control *control = 0;

	try
	{
		Control::capset cp;
		cp.set(Control::cap_canread);

		control = new Control(root, ID(id.interface, id.device, 1, 1),
				-55, 125, "˙C", 2, cp, 0, 0,
				"temp", "temperature sensor");
	}
	catch(minor_exception e)
	{
		Util::dlog("CC ds1621 controls not found: %s\n", e.message.c_str());
	}

	if(control)
		controls.add(control);
}
