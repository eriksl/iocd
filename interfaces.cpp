#include "syslog.h"

#include "interfaces.h"
#include "interface.h"
#include "interface_elv.h"

#include <sstream>
#include <iomanip>
using std::ostringstream;

Interfaces::Interfaces() throw()
{
	_probe_elv();
}

Interfaces::~Interfaces() throw()
{
	iterator_t it;

	for(it = _interfaces.begin(); it != _interfaces.end(); it++)
		delete *it;

	_interfaces.clear();
}

Interfaces::const_iterator_t Interfaces::begin() const throw()
{
	return(_interfaces.begin());
}

Interfaces::const_iterator_t Interfaces::end() const throw()
{
	return(_interfaces.end());
}

void Interfaces::_probe_elv() throw()
{
	int				ix;
	ostringstream	id;
	Interface 		*interface_elv;

	for(ix = 0; ix < 2; ix++)
	{
		id.str("");
		id << "/dev/ttyUSB" << ix;
		interface_elv = 0;

		try
		{
			dlog("probe_elv: trying %s\n", id.str().c_str());
			interface_elv = new InterfaceELV(this, id.str());
			vlog("probe_elv: successful probe for %s\n", id.str().c_str());
			_interfaces.push_back(interface_elv);
			return;
		}
		catch(...)
		{
			delete interface_elv;
			vlog("probe_elv: unsuccessful probe for %s\n", id.str().c_str());
		}
	}
}
