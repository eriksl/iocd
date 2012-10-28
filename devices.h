#ifndef _devices_h_
#define _devices_h_

#include <map>
using std::map;

#include "exception.h"
#include "id.h"

class Device;

class Devices
{
	public:

		friend class InterfaceELV;

		typedef map<ID, Device *>	devices_t;
		typedef devices_t::iterator	iterator;

				Devices()							throw(exception);
		virtual	~Devices()							throw();

		iterator	begin()							throw();
		iterator	end()							throw();
		Device*		find_device(ID id)				throw(exception);

	private:

		void		add(Device *)					throw();
		devices_t	devices;
};

#endif
