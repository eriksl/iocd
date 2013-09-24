#ifndef _device_tmp275_h_
#define _device_tmp275_h_

#include "device.h"
#include "id.h"
#include "exception.h"
#include "util.h"
#include "if_private_data.h"

#include <string>
using std::string;

class Control;
class Interfaces;

class DeviceTMP275 : public Device
{
	friend class InterfaceELV;

	public:
				DeviceTMP275(Interfaces *, ID,
						const InterfacePrivateData *)	throw(exception);
		virtual	~DeviceTMP275()							throw();

	private:

		static	string name_short_static()				throw();
		static	string name_long_static()				throw();

		string	name_short()							throw();
		string	name_long()								throw();

		double	read(Control *)							throw(exception);
		bool	probe()									throw();
		void	find_controls()							throw();
};
#endif
