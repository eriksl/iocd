#ifndef _control_digipicco_h_
#define _control_digipicco_h_

#include "control.h"
#include "interface.h"

#include <string>
using std::string;

#include "exception.h"

class ControlDigipicco : public Control
{
	public:

		typedef enum
		{
			control_temperature,
			control_humidity
		} control_t;

				ControlDigipicco(Controls *parent_controls, const Identity &id,
						double min, double max, string unit, int precision, control_t type) throw(exception);
		virtual	~ControlDigipicco() throw();

		double	read() throw(exception);

	protected:

	private:

		control_t _type;

};
#endif
