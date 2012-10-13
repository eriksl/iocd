#include "interface.h"

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "device.h"

class Interface;

#ifndef _devices_h_
#define _devices_h_

class Devices
{
	public:

		typedef vector<Device *>			devices_t;
		typedef devices_t::const_iterator	const_iterator_t;

				Devices(Interface * interface)											throw(string);
		virtual	~Devices()																throw();

		const_iterator_t	begin()												const	throw();
		const_iterator_t	end()												const	throw();
		Interface *			interface()											const	throw();
		void				add(Device *)												throw();

	protected:

	private:

		typedef devices_t::iterator iterator_t;

		Interface	*_interface;
		devices_t	_devices;
};

#endif
