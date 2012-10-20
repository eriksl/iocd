#include "interface.h"
#include "controls.h"
#include "control.h"
#include "interfaces.h"
#include "interface_elv.h"
#include "device.h"
#include "cppstreams.h"
#include "syslog.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;

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

Interface* Interfaces::find(string id) throw(string)
{
	Interfaces::iterator interface;

	if(id.length() == 8)
		id = id.substr(0, 2);

	if(id.length() != 2)
		throw(string("find(interface): id has invalid length"));

	for(interface = begin(); interface != end(); interface++)
		if((**interface).id().substr(0,2) == id)
			break;

	if(interface == end())
		throw(string("find(interface): interface not found"));

	return(*interface);
}

Device* Interfaces::find_device(string id) throw(string)
{
	Interface *interface = find(id);
	return(interface->devices()->find(id));
}

Control* Interfaces::find_control(string id) throw(string)
{
	Device *device = find_device(id);
	return(device->controls()->find(id));
}

Control* Interfaces::find_control_by_name(string id) throw(string)
{
	Interfaces::iterator	interface;
	Devices::iterator		device;
	Controls::iterator		control;
	bool					found = false;

	for(interface = _interfaces.begin(); interface != _interfaces.end(); interface++)
	{
		for(device = (**interface).devices()->begin(); device != (**interface).devices()->end(); device++)
		{
			for(control = (**device).controls()->begin(); control != (**device).controls()->end(); control++)
			{
				if((**control).shortname() == id)
				{
					found = true;
					break;
				}

				if((**control).longname() == id)
				{
					found = true;
					break;
				}

				if((**control).path() == id)
				{
					found = true;
					break;
				}

				if((**control).id() == id)
				{
					found = true;
					break;
				}
			}
		}
	}

	if(!found)
		throw(string("control not found"));

	return(*control);
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
