#ifndef _interface_usbraw_h_
#define _interface_usbraw_h_

#include <string>
using std::string;

#include "interface.h"
#include "device_usbraw.h"

#include <stdint.h>
#include <libusb.h>

class Interfaces;

class InterfaceUSBraw : public Interface
{
	public:

		friend class Interfaces;
		friend class DeviceUSBraw;

				InterfaceUSBraw(Interfaces *root, ID id)	throw(exception);
		virtual	~InterfaceUSBraw()							throw();

		static string name_short_static()	throw();
		static string name_long_static()	throw();

		string	name_short()										const	throw();
		string	name_long()											const	throw();
		string	interface_id()										const	throw();

	protected:

		libusb_device	**device_list;

	private:

		struct cmd_t
		{
			int						endpoint;
			libusb_device_handle	*handle;
			int						timeout;
			int						length;
			int						transferred;
			uint8_t					*data;
		};

		void	interface_command(void *)									throw(exception);
		void	find_devices()												throw();
		template<class DeviceT> void probe_device(int vendor, int product)	throw();
};

#endif
