#ifndef _device_atmel_h_
#define _device_atmel_h_

#include "device_i2c.h"
#include "id.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class InterfaceELV;
class Control;

class DeviceAtmel : public DeviceI2C
{
	public:

		friend class InterfaceELV;

				DeviceAtmel(Interfaces *root, ID, int address)	throw(exception);
		virtual	~DeviceAtmel()									throw();

		string	name_short()							const	throw();
		string	name_long()								const	throw();

	private:

		enum
		{
			digital_input = 1,
			analog_input,
			digital_output,
			pwm_output
		};

		int		model;
		string	modelname;
		int		version;
		int		revision;

		bool				probe()							throw();
		void				find_controls()					throw();
		Util::byte_array	getcontrol(int cmd)				throw(exception);
		Util::byte_array	query(int cmd, int index, int length,
				int param1 = -1, int param2 = -1,
				int param3 = -1, int param4 = -1)			throw(exception);

		double	read(Control *)						throw(exception);
		void	write(Control *, double value)		throw(exception);
		double	readwrite(Control *, double value)	throw(exception);
		int		readcounter(Control *)				throw(exception);
		int		readresetcounter(Control *)			throw(exception);
		int		readpwmmode(Control *)				throw(exception);
		void	writepwmmode(Control *, int value)	throw(exception);
		string	readpwmmode_string(Control *)		throw(exception);

		Util::byte_array command(string cmd, int timeout = 200,
				int chunks = 1)									throw(exception);

		static string name_short_static()						throw();
		static string name_long_static()						throw();

};
#endif
