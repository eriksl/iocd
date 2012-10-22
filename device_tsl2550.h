#ifndef _device_tsl2550_h_
#define _device_tsl2550_h_

#include "device_i2c.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceTSL2550 : public DeviceI2C
{
	public:
		DeviceTSL2550(Devices*, const Identity &, int address)	throw(exception);
		~DeviceTSL2550()										throw();

	protected:

	private:

		bool	_probe()	throw();
};
#endif
