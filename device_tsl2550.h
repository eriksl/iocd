#ifndef _device_tsl2550_h_
#define _device_tsl2550_h_

#include "device_i2c.h"
#include "util.h"
#include "exception.h"

class Interfaces;
class Control;

class DeviceTSL2550 : public DeviceI2C
{
	public:

		friend class InterfaceELV;

				DeviceTSL2550(Interfaces *root, ID, int address)	throw(exception);
		virtual	~DeviceTSL2550()									throw();

		string	name_short()							const	throw();
		string	name_long()								const	throw();

	private:

		enum sens_t
		{
			sens_low,
			sens_high
		};

		string	name()											throw();
		string	description()									throw();
		double	read(Control *)									throw(exception);
		bool	probe()											throw();
		void	find_controls()									throw();
		double	read_retry(int attempts, sens_t)				throw(exception);
		double	read_sens(sens_t)								throw(exception);
		bool	adc2count(int in, int &out, bool &overflow)		throw();
		double	count2lux(int ch0, int ch1, int multiplier)		throw();

		static	string	name_short_static()						throw();
		static	string	name_long_static()						throw();

};
#endif
