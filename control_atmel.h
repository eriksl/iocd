#include "control.h"
#include "interface.h"
#include "cppstreams.h"

class Device;

#include <string>
using std::string;

#ifndef _control_atmel_digital_input_h_
#define _control_atmel_digital_input_h_

class ControlAtmel : public Control
{
	public:

		typedef enum
		{
			digital_input,
			analog_input,
			digital_output,
			pwm_output
		} control_t;

				ControlAtmel(Interface *parent_interface, Device *parent_device,
						int min, int max, string unit, control_t control_type, int ordinal)	throw(string);
		virtual	~ControlAtmel()																throw();

		int		read()					throw(string);
		void	write(int)				throw(string);
		int		readcounter()			throw(string);
		int		readresetcounter()		throw(string);
		int		readpwmmode()			throw(string);
		void	writepwmmode(int)		throw(string);

	protected:

	private:

		control_t				_control_type;
		int						_ordinal;

		Interface::byte_array	_query(int cmd, int length, int param1 = -1, int param2 = -1, int param3 = -1, int param4 = -1) throw(string);
};
#endif
