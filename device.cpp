#include "interfaces.h"
#include "interface.h"
#include "device.h"

Device::Device(Interfaces *root_in, ID id_in) throw(exception)
	:
		id(id_in),
		controls(),
		root(root_in)
{
}

Device::~Device() throw()
{
}

void Device::command(void *cmd) throw(exception)
{
	parent()->command(cmd);
}

double Device::read(Control *) throw(exception)
{
	throw(minor_exception("device: unimplemented: read"));
}

void Device::write(Control *, double) throw(exception)
{
	throw(minor_exception("device: unimplemented: write"));
}

double Device::readwrite(Control *, double) throw(exception)
{
	throw(minor_exception("device: unimplemented: readwrite"));
}

int Device::readcounter(Control *) throw(exception)
{
	throw(minor_exception("device: unimplemented: readcounter"));
}

int Device::readresetcounter(Control *) throw(exception)
{
	throw(minor_exception("device: unimplemented: readresetcounter"));
}

int Device::readpwmmode(Control *) throw(exception)
{
	throw(minor_exception("device: unimplemented: readpwmmode"));
}

void Device::writepwmmode(Control *, int) throw(exception)
{
	throw(minor_exception("device: unimplemented: writepwmmode"));
}

string Device::readpwmmode_string(Control *) throw(exception)
{
	throw(minor_exception("device: unimplemented: readpwmmode_string"));
}

Interface* Device::parent() throw(exception)
{
	return(root->find_interface(id));
}

Controls* Device::device_controls() throw()
{
	return(&controls);
}
