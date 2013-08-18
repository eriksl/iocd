#include "interface.h"
#include "control.h"
#include "interfaces.h"
//#include "interface_elv.h"
#include "interface_usbraw.h"
#include "device.h"
#include "cppstreams.h"
#include "util.h"

#include <signal.h>

Interfaces::Interfaces() throw(exception)
	:
		enumerator(1),
		signal_value(Interfaces::signal_none)
{
	pthread_cond_init(&signal_condition, 0);
	pthread_mutex_init(&signal_mutex, 0);

	instance = this;
	::signal(SIGINT, sigint);
	::signal(SIGQUIT, sigquit);

	probe_interfaces();
}

Interfaces::~Interfaces() throw()
{
	::signal(SIGINT, SIG_DFL);
	::signal(SIGQUIT, SIG_DFL);

	clear();

	pthread_mutex_destroy(&signal_mutex);
	pthread_cond_destroy(&signal_condition);
}

Interfaces::iterator Interfaces::begin() throw()
{
	return(interfaces.begin());
}

Interfaces::iterator Interfaces::end() throw()
{
	return(interfaces.end());
}

size_t Interfaces::count() throw()
{
	return(interfaces.size());
}

void Interfaces::clear() throw()
{
	iterator it;

	for(it = interfaces.begin(); it != interfaces.end(); it++)
		delete it->second;

	interfaces.clear();
}

void Interfaces::remove(ID id) throw()
{
	interfaces_t::iterator it;

	try
	{
		if((it = interfaces.find(id)) == interfaces.end())
			return;
	}
	catch(...)
	{
		return;
	}

	try
	{
		interfaces.erase(it);
	}
	catch(...)
	{
	}
}

Interface* Interfaces::find_interface(ID id) throw(exception)
{
	interfaces_t::iterator it;

	id.device			= 0;
	id.control_type		= 0;
	id.control_index	= 0;

	if((it = interfaces.find(id)) == interfaces.end())
		throw(minor_exception("find_interface: interface not found"));

	return(it->second);
}

Device* Interfaces::find_device(ID id) throw(exception)
{
	Interface *interface = find_interface(id);
	return(interface->devices.find_device(id));
}

Control* Interfaces::find_control(ID id) throw(exception)
{
	Device *device = find_device(id);
	return(device->controls.find(id));
}

Interfaces::signal_t Interfaces::wait() throw()
{
	signal_t rv;

	pthread_mutex_lock(&signal_mutex);
	pthread_cond_wait(&signal_condition, &signal_mutex);
	rv = signal_value;
	pthread_mutex_unlock(&signal_mutex);

	return(rv);
}

void Interfaces::signal(Interfaces::signal_t value) throw()
{
	pthread_mutex_lock(&signal_mutex);
	signal_value = value;
	pthread_cond_signal(&signal_condition);
	pthread_mutex_unlock(&signal_mutex);
}

void Interfaces::probe_interfaces() throw()
{
	//probe_interface_1<InterfaceELV>("/dev/elv");
	probe_interface_0<InterfaceUSBraw>();
}

template<class InterfaceT> void Interfaces::probe_interface_0() throw()
{
	InterfaceT *interface = 0;
	ID			id(enumerator++);
	string		error;

	try
	{
		Util::dlog("** probing for %s\n", InterfaceT::name_short_static().c_str());
		interface = new InterfaceT(this, id); 
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
		Util::dlog("** probe for %s successful\n", interface->interface_id().c_str());
		interfaces[id] = interface;
		interface->probe_all_devices();
	}
	else
		Util::dlog("** probe for %s unsuccessful\n", InterfaceT::name_short_static().c_str());
}

template<class InterfaceT> void Interfaces::probe_interface_1(string device_node) throw()
{
	InterfaceT *interface = 0;
	ID			id(enumerator++);
	string		error;

	try
	{
		Util::dlog("** probing for %s:%s\n", InterfaceT::name_short_static().c_str(), device_node.c_str());
		interface = new InterfaceT(this, id, device_node); 
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
		Util::dlog("** probe for %s successful\n", interface->interface_id().c_str());
		interfaces[id] = interface;
		interface->find_devices();
	}
	else
		Util::dlog("** probe %s at %s unsuccessful\n", InterfaceT::name_short_static().c_str(), device_node.c_str());
}

void Interfaces::sigint(int)
{
	Util::dlog("SIGINT\n");
	::signal(SIGINT, SIG_DFL);

	Interfaces::instance->signal(Interfaces::signal_user_keyint);
}

void Interfaces::sigquit(int)
{
	Util::dlog("SIGQUIT\n");
	::signal(SIGQUIT, SIG_DFL);

	Interfaces::instance->signal(Interfaces::signal_user_keyquit);
}

Interfaces* Interfaces::instance = 0;
