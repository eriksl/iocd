#ifndef _device_digipicco_h_
#define _device_digipicco_h_

#include "device_i2c.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceDigipicco : public DeviceI2C
{
	public:
		DeviceDigipicco(Devices*, const Identity&, int address)		throw(exception);
		~DeviceDigipicco()											throw();
	
	protected:

	private:

		bool	_probe()	throw();
};
#endif
