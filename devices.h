#ifndef _devices_h_
#define _devices_h_

#include "interface.h"
#include "identity.h"

#include <string>
using std::string;

#include <vector>
using std::vector;

class Interface;
class Device;

class Devices
{
	public:

		typedef vector<Device *>	devices_t;
		typedef devices_t::iterator	iterator;

				Devices(Interface *interface)	throw(string);
		virtual	~Devices()						throw();

		iterator	begin()						throw();
		iterator	end()						throw();
		void		add(Device *)				throw();
		Device*		find(string id)				throw(string);
		Interface*	interface()					throw();

	protected:

		Interface	*_interface;
		devices_t	_devices;

	private:
};

#endif
