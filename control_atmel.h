#ifndef _control_atmel_h_
#define _control_atmel_h_

#include "control.h"
#include "util.h"

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
						double min, double max, string unit, int precision,
						control_t control_type, int index)	throw(exception);
		virtual	~ControlAtmel()								throw();

		double	read()					throw(exception);
		void	write(double)			throw(exception);
		int		readcounter()			throw(exception);
		int		readresetcounter()		throw(exception);
		int		readpwmmode()			throw(exception);
		void	writepwmmode(int)		throw(exception);
		string	readpwmmode_string()	throw(exception);

	protected:

	private:

		control_t			_control_type;
		int					_index;

		Util::byte_array	_query(int cmd, int length, int param1 = -1, int param2 = -1, int param3 = -1, int param4 = -1) throw(exception);
};
#endif
