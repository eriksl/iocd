#ifndef _device_atmel_h_
#define _device_atmel_h_

#include "device.h"
#include "id.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class Control;
class Interfaces;

class DeviceAtmel : public Device
{
	friend class Interface;
	friend class InterfaceUSBraw;

	public:

				DeviceAtmel(Interfaces *root, ID, void *pdata)	throw(exception);
		virtual	~DeviceAtmel()									throw();

		string	name_short()									throw();
		string	name_long()										throw();

	private:

		enum
		{
			digital_input = 1,
			analog_input,
			digital_output,
			pwm_output,
			temp_sensor
		};

		int		model;
		string	modelname;
		int		version;
		int		revision;

		double	read(Control *)								throw(exception);
		void	write(Control *, double value)				throw(exception);
		double	readwrite(Control *, double value)			throw(exception);
		int		readcounter(Control *)						throw(exception);
		int		readresetcounter(Control *)					throw(exception);
		int		readpwmmode(Control *)						throw(exception);
		void	writepwmmode(Control *, int value)			throw(exception);
		string	readpwmmode_string(Control *)				throw(exception);

		static string name_short_static()					throw();
		static string name_long_static()					throw();

		bool		probe()									throw();
		void		find_controls()							throw(exception);

		ByteArray	command(int expected_length, ByteArray)	throw(exception); 
		ByteArray	command(int expected_length,
				int a, int b = -1, int c = -1)				throw(exception); 
};
#endif
