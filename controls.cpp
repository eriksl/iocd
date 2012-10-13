#include "syslog.h"

#include "controls.h"
#include "control.h"

Controls::Controls(Interface * parent_interface, Device * parent_device) throw() :
	_interface(parent_interface),
	_device(parent_device)
{
}

Controls::~Controls() throw()
{
	iterator_t it;

	for(it = _controls.begin(); it != _controls.end(); it++)
		delete *it;

	_controls.clear();
}

Controls::const_iterator_t Controls::begin() const throw()
{
	return(_controls.begin());
}

Controls::const_iterator_t Controls::end() const throw()
{
	return(_controls.end());
}

void Controls::add(Control * new_control) throw()
{
	_controls.push_back(new_control);
}
