#ifndef _controls_h_
#define _controls_h_

#include <vector>
using std::vector;

class Device;
class Control;

class Controls
{
	public:

		typedef vector<Control *>		controls_t;
		typedef controls_t::iterator	iterator;

				Controls(Device *)		throw();
		virtual ~Controls()				throw();

		iterator	begin()				throw();
		iterator	end()				throw();
		void		add(Control *)		throw();
		Control *	find(string id)		throw(string);
		Device*		device()			throw();

	protected:

	private:

		Device		*_device;
		controls_t	_controls;
};

#endif
