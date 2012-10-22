#ifndef _device_i2c_h_
#define _device_i2c_h_

#include "device.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class DeviceI2C : public Device
{
	public:

				DeviceI2C(Devices *parent, const Identity& id, int address)				throw(exception);
		virtual	~DeviceI2C()															throw();
		virtual	Util::byte_array command(string cmd, int timeout = 200, int chunks = 1)	throw(exception);

	protected:

		int		_address;

	private:
};
#endif
