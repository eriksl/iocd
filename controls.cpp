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

Control* Controls::find(string id) throw(string)
{
	Controls::iterator control;

	if(id.length() == 8)
		id = id.substr(4,2);

	if(id.length() != 2)
		throw(string("find(control): id has invalid length"));

	for(control = begin(); control != end(); control++)
		if((**control).id().substr(4,2) == id)
			break;

	if(control == end())
		throw(string("find(control): control not found"));

	return(*control);
}

Device *Controls::device() throw()
{
	return(_device);
}
