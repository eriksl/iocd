#include "control.h"
#include "controls.h"

Controls::Controls() throw()
{
}

Controls::~Controls() throw()
{
	iterator it;

	for(it = controls.begin(); it != controls.end(); it++)
		delete it->second;

	controls.clear();
}

Controls::iterator Controls::begin() throw()
{
	return(controls.begin());
}

Controls::iterator Controls::end() throw()
{
	return(controls.end());
}

void Controls::add(Control* control) throw()
{
	controls[control->id] = control;
}

Control* Controls::find(ID id) throw(exception)
{
	Controls::iterator it;;

	if((it = controls.find(id)) == controls.end())
		throw(minor_exception("find control: not found"));

	return(it->second);
}
