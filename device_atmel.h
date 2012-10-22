#ifndef _device_atmel_h_
#define _device_atmel_h_

#include "device_i2c.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class DeviceAtmel : public DeviceI2C
{
	public:
		DeviceAtmel(Devices*, const Identity &, int address)	throw(exception);
		~DeviceAtmel()											throw();

		Util::byte_array command(string cmd, int timeout = 200,
				int chunks = 1)									throw(exception);
	
	protected:

	private:

		int		_model;
		string	_modelname;
		int		_version;
		int		_revision;

		bool				_probe()							throw();
		Util::byte_array	_getcontrol(int cmd)				throw(exception);
};
#endif
