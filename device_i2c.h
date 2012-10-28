#ifndef _device_i2c_h_
#define _device_i2c_h_

#include "device.h"
#include "id.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class Interfaces;

class DeviceI2C : public Device
{
	public:

				DeviceI2C(Interfaces *root, ID, int address)							throw(exception);
		virtual	Util::byte_array command(string cmd, int timeout = 200, int chunks = 1)	throw(exception);

				string	device_id()												const	throw();

	protected:

		int		address;

	private:

		virtual	string	name_short()											const	throw() = 0;
		virtual	string	name_long()												const	throw() = 0;
};
#endif
