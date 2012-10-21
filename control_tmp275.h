#ifndef _control_tmp275_h_
#define _control_tmp275_h_

#include "control.h"
#include "interface.h"

#include <string>
using std::string;

#include "exception.h"

class ControlTMP275 : public Control
{
	public:

				ControlTMP275(Controls *parent_controls,
						int generation, int parent_id, int ordinal, string parent_path,
						double min, double max, string unit, int precision) throw(exception);
		virtual	~ControlTMP275() throw();

		double	read() throw(exception);

	protected:

	private:
};
#endif
