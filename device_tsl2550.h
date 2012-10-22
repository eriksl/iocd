#ifndef _device_tsl2550_h_
#define _device_tsl2550_h_

#include "device.h"
#include "interface.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceTSL2550 : public Device
{
	public:
		DeviceTSL2550(Devices*, const Identity &, int address)	throw(exception);
		~DeviceTSL2550()										throw();

		Interface::byte_array command(string cmd,
						int timeout = 200, int chunks = 1)	const	throw(exception);
	
	protected:

	private:

		int		_address;
		bool	_probe()	throw();
};
#endif
