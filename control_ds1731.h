#ifndef _control_ds1731_h_
#define _control_ds1731_h_

#include "control.h"
#include "interface.h"

#include <string>
using std::string;

#include "exception.h"

class ControlDS1731 : public Control
{
	public:

				ControlDS1731(Controls *parent_controls, const Identity &id,
						double min, double max, string unit, int precision) throw(exception);
		virtual	~ControlDS1731() throw();

		double	read() throw(exception);

	protected:

	private:
};
#endif
