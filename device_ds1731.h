#ifndef _device_ds1731_h_
#define _device_ds1731_h_

#include "device_i2c.h"
#include "id.h"
#include "exception.h"

#include <string>
using std::string;

class Interfaces;
class Control;

class DeviceDS1731 : public DeviceI2C
{
	public:

		friend class InterfaceELV;

				DeviceDS1731(Interfaces *root, ID, int address)	throw(exception);
		virtual	~DeviceDS1731()									throw();

		string	name_short()					const	throw();
		string	name_long()						const	throw();

	private:

		double	read(Control *)							throw(exception);
		bool	probe()									throw();
		void	find_controls()							throw(exception);

		static string name_short_static()				throw();
		static string name_long_static()				throw();

};
#endif
