#ifndef _device_digipicco_h_
#define _device_digipicco_h_

#include "device_i2c.h"
#include "id.h"
#include "util.h"
#include "exception.h"

#include <string>
using std::string;

class Control;

class DeviceDigipicco : public DeviceI2C
{
	public:

		friend class InterfaceELV;

		DeviceDigipicco(Interfaces *root, ID, int address)	throw(exception);
		~DeviceDigipicco()									throw();

		string	name_short()						const	throw();
		string	name_long()							const	throw();

	private:

		enum
		{
			temperature = 1,
			humidity
		};

		double	read(Control *)										throw(exception);
		bool	probe()												throw();
		void	find_controls()										throw();

		static string name_short_static()							throw();
		static string name_long_static()							throw();
	
};
#endif
