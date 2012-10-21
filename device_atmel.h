#ifndef _device_atmel_h_
#define _device_atmel_h_

#include "device.h"
#include "interface.h"
#include "exception.h"

#include <string>
using std::string;

class DeviceAtmel : public Device
{
	public:
		DeviceAtmel(Devices*, int generation, int parent_id, int ordinal,
					string parent_path, int address)		throw(exception);
		~DeviceAtmel()										throw();

		Interface::byte_array command(string cmd,
						int timeout = 200, int chunks = 1)	const	throw(exception);
	
	protected:

	private:

		int		_address;
		int		_model;
		string	_modelname;
		int		_version;
		int		_revision;

		bool					_probe()						throw();
		Interface::byte_array	_getcontrol(int cmd)	const	throw(exception);
};
#endif
