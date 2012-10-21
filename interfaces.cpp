#include "interface.h"
#include "controls.h"
#include "control.h"
#include "interfaces.h"
#include "interface_elv.h"
#include "device.h"
#include "cppstreams.h"
#include "syslog.h"

#include <signal.h>

Interfaces::Interfaces() throw(exception)
	:	_enumerator(1), _signal_value(Interfaces::signal_none)
{
	pthread_cond_init(&_signal_condition, 0);
	pthread_mutex_init(&_signal_mutex, 0);

	__instance = this;
	::signal(SIGINT, __sigint);
	::signal(SIGQUIT, __sigquit);

	_probe();
}

Interfaces::~Interfaces() throw()
{
	::signal(SIGINT, SIG_DFL);
	::signal(SIGQUIT, SIG_DFL);

	iterator it;

	for(it = _interfaces.begin(); it != _interfaces.end(); it++)
		delete *it;

	_interfaces.clear();

	pthread_mutex_destroy(&_signal_mutex);
	pthread_cond_destroy(&_signal_condition);
}

Interfaces::iterator Interfaces::begin() throw()
{
	return(_interfaces.begin());
}

Interfaces::iterator Interfaces::end() throw()
{
	return(_interfaces.end());
}

size_t Interfaces::count() throw()
{
	return(_interfaces.size());
}

Interface* Interfaces::find(string id) throw(exception)
{
	Interfaces::iterator interface;

	if(id.length() == 8)
		id = id.substr(0, 2);

	if(id.length() != 2)
		throw(minor_exception("find(interface): id has invalid length"));

	for(interface = begin(); interface != end(); interface++)
		if((**interface).id().substr(0,2) == id)
			break;

	if(interface == end())
		throw(minor_exception("find(interface): interface not found"));

	return(*interface);
}

Device* Interfaces::find_device(string id) throw(exception)
{
	Interface *interface = find(id);
	return(interface->devices()->find(id));
}

Control* Interfaces::find_control(string id) throw(exception)
{
	Device *device = find_device(id);
	return(device->controls()->find(id));
}

Interfaces::signal_t Interfaces::wait() throw()
{
	signal_t rv;

	pthread_mutex_lock(&_signal_mutex);
	pthread_cond_wait(&_signal_condition, &_signal_mutex);
	rv = _signal_value;
	pthread_mutex_unlock(&_signal_mutex);

	return(rv);
}

void Interfaces::signal(Interfaces::signal_t value) throw()
{
	pthread_mutex_lock(&_signal_mutex);
	_signal_value = value;
	pthread_cond_signal(&_signal_condition);
	pthread_mutex_unlock(&_signal_mutex);
}

Control* Interfaces::find_control_by_name(string id) throw(exception)
{
	Interfaces::iterator	interface;
	Devices::iterator		device;
	Controls::iterator		control;

	for(interface = _interfaces.begin(); interface != _interfaces.end(); interface++)
	{
		for(device = (**interface).devices()->begin(); device != (**interface).devices()->end(); device++)
		{
			for(control = (**device).controls()->begin(); control != (**device).controls()->end(); control++)
			{
				if((**control).shortname() == id)
					return(*control);

				if((**control).longname() == id)
					return(*control);

				if((**control).path() == id)
					return(*control);

				if((**control).id() == id)
					return(*control);
			}
		}
	}

	throw(minor_exception("control not found"));

	return(0); // never reached
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
		catch(minor_exception e)
		{
			error = e.message;
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

void Interfaces::__sigint(int)
{
	dlog("SIGINT\n");
	::signal(SIGINT, SIG_DFL);

	Interfaces::__instance->signal(Interfaces::signal_user_keyint);
}

void Interfaces::__sigquit(int)
{
	dlog("SIGQUIT\n");
	::signal(SIGQUIT, SIG_DFL);

	Interfaces::__instance->signal(Interfaces::signal_user_keyquit);
}

Interfaces* Interfaces::__instance = 0;
