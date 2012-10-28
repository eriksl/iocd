#ifndef _device_tmp275_h_
#define _device_tmp275_h_

#include "device_i2c.h"
#include "id.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class Interfaces;
class Control;

class DeviceTMP275 : public DeviceI2C
{
	friend class InterfaceELV;

	public:
				DeviceTMP275(Interfaces *, ID,  int address)	throw(exception);
				~DeviceTMP275()									throw();

		string	name_short()							const	throw();
		string	name_long()								const	throw();

	private:

		static	string name_short_static()						throw();
		static	string name_long_static()						throw();

		bool	probe()											throw();
		void	find_controls()									throw();
		double	read(Control *)									throw(exception);
};
#endif
