#include "device_tmp275.h"
#include "control.h"
#include "id.h"
#include "cppstreams.h"
#include "util.h"

#include <unistd.h>

DeviceTMP275::DeviceTMP275(Interfaces *root_in, ID id_in, int address_in) throw(exception)
	:
		DeviceI2C(root_in, id_in, address_in)
{
	if(!probe())
		throw(minor_exception(string("tmp275 not detected at ") + Util::int_to_string(address)));
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

string DeviceTMP275::name_short() const throw()
{
	return(name_short_static());
}

string DeviceTMP275::name_long() const throw()
{
	return(name_long_static());
}

double DeviceTMP275::read(Control *) throw(exception)
{
	Util::byte_array	bytes;
	int16_t				tmp;

	try
	{
		// put device in 12 bits / shutdown mode, take one ("one shot") measurement
		bytes = command("w 01 e1 r 01");

		if(bytes.size() != 1)
			throw(minor_exception("invalid reply size from control"));

		if(bytes[0] != 0x61)
			throw(minor_exception("invalid reply from control"));

		// conversion time takes 300 ms max
		usleep(300000);

		bytes = command("w 00 r 02");

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

bool DeviceTMP275::probe() throw()
{
	stringstream		conv;
	Util::byte_array	in;

	try
	{
		// write 0xabcd in threshold register,
		// check treshold register, it should return 0xabc0

		in = command("w 02 ab cd r 02");

		if(in.size() != 2)
			throw(minor_exception("incorrect length in reply"));

		if((in[0] != 0xab) || (in[1] != 0xc0))
			throw(minor_exception("incorrect reply from probe"));

		in = command("w 01 e1 r 01");

		if(in.size() != 1)
			throw(minor_exception("incorrect length in reply"));

		if(in[0] != 0x61)
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
