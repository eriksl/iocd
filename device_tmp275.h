#ifndef _device_tmp275_h_
#define _device_tmp275_h_

#include "device.h"
#include "interface.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceTMP275 : public Device
{
	public:
		DeviceTMP275(Devices*, const Identity&,  int address)	throw(exception);
		~DeviceTMP275()											throw();

		Interface::byte_array command(string cmd,
						int timeout = 200, int chunks = 1)	const	throw(exception);
	
	protected:

	private:

		int		_address;
		bool	_probe()	throw();
};
#endif
