#ifndef _device_digipicco_h_
#define _device_digipicco_h_

#include "device.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceDigipicco : public Device
{
	public:
		DeviceDigipicco(Devices*, const Identity&, int address)		throw(exception);
		~DeviceDigipicco()											throw();

		Util::byte_array command(string cmd, int timeout = 200,
				int chunks = 1)								const	throw(exception);
	
	protected:

	private:

		int		_address;
		bool	_probe()	throw();
};
#endif
