#ifndef _device_ds1731_h_
#define _device_ds1731_h_

#include "device.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceDS1731 : public Device
{
	public:
		DeviceDS1731(Devices*, const Identity&, int address)	throw(exception);
		~DeviceDS1731()											throw();

		Util::byte_array command(string cmd,
						int timeout = 200, int chunks = 1)	const	throw(exception);
	
	protected:

	private:

		int		_address;
		bool	_probe()	throw();
};
#endif
