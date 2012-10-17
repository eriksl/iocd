#include "control.h"
#include "controls.h"
#include "syslog.h"

Controls::Controls(Device *parent_device) throw()
	:	_device(parent_device)
{
}

Controls::~Controls() throw()
{
	iterator it;

	for(it = _controls.begin(); it != _controls.end(); it++)
		delete *it;

	_controls.clear();
}

Controls::iterator Controls::begin() throw()
{
	return(_controls.begin());
}

Controls::iterator Controls::end() throw()
{
	return(_controls.end());
}

void Controls::add(Control* new_control) throw()
{
	_controls.push_back(new_control);
}

Device *Controls::device() throw()
{
	return(_device);
}
