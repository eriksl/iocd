#include "device_digipicco.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

DeviceDigipicco::DeviceDigipicco(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:
		DeviceI2C(root_in, id_in, address_in)
{
	if(!probe())
		throw(minor_exception(string("no digipicco device found at ") + Util::int_to_string(address)));
}

DeviceDigipicco::~DeviceDigipicco() throw()
{
}

string DeviceDigipicco::name_short_static() throw()
{
	return("digipicco");
}

string DeviceDigipicco::name_long_static() throw()
{
	return("Digipicco temperature and humidity sensor");
}

string DeviceDigipicco::name_short() const throw()
{
	return(name_short_static());
}

string DeviceDigipicco::name_long() const throw()
{
	return(name_long_static());
}

double DeviceDigipicco::read(Control *control) throw(exception)
{
	Util::byte_array	bytes;
	uint16_t			rv[2];
	int					attempt;
	double				temphum;

	try
	{
		for(attempt = 0; attempt < 3; attempt++)
		{
			bytes = command("r 04 ");

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

	switch(control->type)
	{
		case(DeviceDigipicco::temperature):
		{
			temphum = (((double)rv[1] / 32767) * 165) - 40;
			break;
		}

		case(DeviceDigipicco::humidity):
		{
			temphum = (double)rv[0] / 327.67;
			break;
		}

		default:
			throw(minor_exception("digipicco: invalid control type"));
	}

	return(temphum);
}

bool DeviceDigipicco::probe() throw()
{
	stringstream			conv;
	Util::byte_array		in;
	int						attempt;

	try
	{
		for(attempt = 0; attempt < 3; attempt++)
		{
			in = command("r 04 ");

			if(in.size() == 4)
				break;

			usleep(100000);
		}

		if(attempt > 2)
			throw(minor_exception("incorrect length in reply"));

		if((in[0] == 0xff) && (in[1] == 0xff) && (in[2] == 0xff) && (in[3] == 0xff))
			throw(minor_exception("incorrect reply"));
	}
	catch(minor_exception e)
	{
		Util::dlog("digipicco not found: %s\n", e.message.c_str());
		return(false);
	}

	Util::dlog("probe: digipicco detected\n");
	return(true);
}

void DeviceDigipicco::find_controls() throw()
{
	ostringstream	control_name_short, control_name_long;
	Control			*control_hum = 0;
	Control			*control_temp = 0;
	Control::capset	cc;

	cc.set(Control::cap_canread);

	try
	{
		control_temp = new Control(root, ID(id.interface, id.device, int(DeviceDigipicco::temperature), 1),
					-40, 125, "˙C", 2, cc, DeviceDigipicco::temperature, 0,
					"temp", "Temperature sensor");

		control_hum = new Control(root, ID(id.interface, id.device, int(DeviceDigipicco::humidity), 1), 
					0, 100, "%", 0, cc, DeviceDigipicco::humidity, 0,
					"hum", "humidity sensor");
	}
	catch(minor_exception e)
	{
		Util::dlog("CC digipicco: %s\n", e.message.c_str());
	}

	if(control_hum)
		controls.add(control_hum);

	if(control_temp)
		controls.add(control_temp);
}
