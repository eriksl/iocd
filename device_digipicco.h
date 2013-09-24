#ifndef _device_digipicco_h_
#define _device_digipicco_h_

#include "device.h"
#include "id.h"
#include "exception.h"
#include "util.h"
#include "if_private_data.h"

#include <string>
using std::string;

class Control;
class Interfaces;

class DeviceDigipicco : public Device
{
	friend class InterfaceELV;

	public:

				DeviceDigipicco(Interfaces *root, ID, 
						const InterfacePrivateData *)		throw(exception);
		virtual	~DeviceDigipicco()							throw();

	private:

		enum
		{
			temperature = 1,
			humidity
		};

		static string name_short_static()					throw();
		static string name_long_static()					throw();

		string	name_short()								throw();
		string	name_long()									throw();

		double	read(Control *)								throw(exception);
		bool	probe()										throw();
		void	find_controls()								throw();
};
#endif
