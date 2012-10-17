#ifndef _control_atmel_h_
#define _control_atmel_h_

#include "control.h"
#include "interface.h"

#include <string>
using std::string;

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

				ControlAtmel(Controls *parent_controls,
						int generation, int parent_id, int ordinal, string parent_path,
						int min, int max, string unit,
						control_t control_type, int index)	throw(string);
		virtual	~ControlAtmel()								throw();

		int		read()					throw(string);
		void	write(int)				throw(string);
		int		readcounter()			throw(string);
		int		readresetcounter()		throw(string);
		int		readpwmmode()			throw(string);
		void	writepwmmode(int)		throw(string);

	protected:

	private:

		control_t				_control_type;
		int						_index;

		Interface::byte_array	_query(int cmd, int length, int param1 = -1, int param2 = -1, int param3 = -1, int param4 = -1) throw(string);
};
#endif
