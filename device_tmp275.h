#ifndef _device_tmp275_h_
#define _device_tmp275_h_

#include "device_i2c.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class DeviceTMP275 : public DeviceI2C
{
	public:
		DeviceTMP275(Devices*, const Identity&,  int address)	throw(exception);
		~DeviceTMP275()											throw();

	protected:

	private:

		bool	_probe()	throw();
};
#endif
