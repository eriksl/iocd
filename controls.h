#include "control.h"

#include <vector>
using std::vector;

class Interface;
class Device;

#ifndef _controls_h_
#define _controls_h_

class Controls
{
	public:

		typedef vector<Control *>			controls_t;
		typedef controls_t::const_iterator	const_iterator_t;

				Controls(Interface *, Device *)	throw();
		virtual ~Controls()	throw();

		const_iterator_t	begin()		const	throw();
		const_iterator_t	end()		const	throw();
		void				add(Control *)		throw();

	protected:

	private:

		typedef controls_t::iterator iterator_t;

		controls_t	_controls;
		Interface	*_interface;
		Device		*_device;
};

#endif
