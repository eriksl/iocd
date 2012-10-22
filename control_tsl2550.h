#ifndef _control_tsl2550_h_
#define _control_tsl2550_h_

#include "control.h"
#include "interface.h"

#include <string>
using std::string;

#include "exception.h"

class ControlTSL2550 : public Control
{
	public:

				ControlTSL2550(Controls *parent_controls,
						int generation, int parent_id, int ordinal, string parent_path,
						double min, double max, string unit, int precision) throw(exception);
		virtual	~ControlTSL2550() throw();

		double	read() throw(exception);

	protected:

		double	_read_retry(int attempts, bool erange) throw(exception);
		double	_read_range(bool erange) throw(exception);
		bool	_adc2count(int in, int &out, bool &overflow);
		double	_count2lux(int ch0, int ch1, int multiplier);

	private:
};
#endif
