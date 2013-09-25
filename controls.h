#ifndef _controls_h_
#define _controls_h_

#include <map>
using std::map;

#include "id.h"
#include "exception.h"

class Interfaces;
class Device;
class Control;

class Controls
{
	public:

		friend class DeviceAtmel;
		friend class DeviceDigipicco;
		friend class DeviceTMP275;
		friend class DeviceTSL2550;
		friend class DeviceK8055;
		friend class DeviceDS1731;
		friend class DeviceDS1621;

		typedef map<ID, Control *>		controls_t;
		typedef controls_t::iterator	iterator;

				Controls()					throw();
		virtual ~Controls()					throw();

		iterator	begin()					throw();
		iterator	end()					throw();
		Control*	find(ID id)				throw(exception);

	private:

		void		add(Control *)			throw();
		controls_t	controls;
};

#endif
