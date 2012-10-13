#include "device.h"
#include "interface.h"

#include <string>
using std::string;

#ifndef _device_atmel_h_
#define _device_atmel_h_

class DeviceAtmel : public Device
{
	public:
				DeviceAtmel(Interface*, Devices*, int address)		throw(string);
		virtual	~DeviceAtmel()										throw();

		Interface::byte_array command(string cmd,
						int timeout = 200, int chunks = 1)	const	throw(string);
	
	protected:

	private:

		int		_address;
		int		_model;
		string	_modelname;
		int		_version;
		int		_revision;

		bool					_probe()						throw();
		Interface::byte_array	_getcontrol(int cmd)	const	throw(string);
};
#endif
