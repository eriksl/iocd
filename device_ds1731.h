#ifndef _device_ds1731_h_
#define _device_ds1731_h_

#include "device_i2c.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceDS1731 : public DeviceI2C
{
	public:
		DeviceDS1731(Devices*, const Identity&, int address)	throw(exception);
		~DeviceDS1731()											throw();

	protected:

	private:

		bool	_probe()	throw();
};
#endif
