#include "controls.h"
#include "interface.h"

#include <string>
using std::string;

class Devices;

#ifndef _device_h_
#define _device_h_

class Device
{
	public:

				Device(Interface *, Devices *)					throw(string);
		virtual	~Device()										throw();
		virtual Interface::byte_array command(string cmd,
				int timeout = 200, int chunks = 1)		const	throw(string) = 0;

		string		name()								const	throw();
		string		bus()								const	throw();
		Interface *	interface()							const	throw();
		Devices *	devices()							const	throw();
		Controls *	controls()							const	throw();

	protected:

		Interface	*_interface;
		Devices		*_devices;
		Controls	*_controls;
		string		_name;
		string		_bus;

	private:
};
#endif
