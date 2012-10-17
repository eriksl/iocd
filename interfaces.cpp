#include "interfaces.h"
#include "interface.h"
#include "interface_elv.h"
#include "cppstreams.h"
#include "syslog.h"

Interfaces::Interfaces() throw(string)
	:	_enumerator(1)
{
	_probe();
}

Interfaces::~Interfaces() throw()
{
	iterator it;

	for(it = _interfaces.begin(); it != _interfaces.end(); it++)
		delete *it;

	_interfaces.clear();
}

Interfaces::iterator Interfaces::begin() throw()
{
	return(_interfaces.begin());
}

Interfaces::iterator Interfaces::end() throw()
{
	return(_interfaces.end());
}

void Interfaces::_probe() throw()
{
	_probe_usb();
}

void Interfaces::_probe_usb() throw()
{
	_probe_usb_elv();
}

void Interfaces::_probe_usb_elv() throw()
{
	int				ix;
	stringstream	rid;
	InterfaceELV	*interface;
	string			error;

	dlog("probing elv interfaces\n");

	for(ix = 0; ix < 2; ix++)
	{
		rid.str("");
		rid << "/dev/ttyUSB" << ix;
		interface = 0;

		try
		{
			dlog("probe elv: trying %s\n", rid.str().c_str());
			interface = new InterfaceELV(this, 0, 0, _enumerator, "", rid.str());
		}
		catch(string e)
		{
			error = e;
		}
		catch(...)
		{
			error = "<unspecified error>";
		}

		if(interface)
		{
			dlog("probe elv: successful probe for %s (%s)\n", interface->shortname().c_str(), interface->id().c_str());
			_interfaces.push_back(interface);
			_enumerator++;
		}
		else
			dlog("probe elv: unsuccesful probe for %s\n", rid.str().c_str());
	}
}
