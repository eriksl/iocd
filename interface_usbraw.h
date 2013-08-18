#ifndef _interface_usbraw_h_
#define _interface_usbraw_h_

#include <string>
using std::string;

#include "interface.h"

#include <stdint.h>
#include <libusb-1.0/libusb.h>

class Interfaces;

class InterfaceUSBraw : public Interface
{
	friend class Interfaces;

	public:

				InterfaceUSBraw(Interfaces *root, ID id)	throw(exception);
		virtual	~InterfaceUSBraw()							throw();

		static string name_short_static()	throw();
		static string name_long_static()	throw();

		string	name_short()	throw();
		string	name_long()		throw();
		string	interface_id()	throw();

	private:

		struct if_usbraw_pdata_t
		{
			libusb_device				*device;
			libusb_device_descriptor	*descriptor;
			libusb_device_handle		*handle;
			uint8_t						write_endpoint;
			uint8_t						read_endpoint;
		};

		template<class DeviceT> void probe_single_device(
				uint8_t write_endpoint, uint8_t read_endpoint,
				int vendor, int product, int version = -1)					throw();

		virtual	void	probe_all_devices()									throw(exception);
		virtual	string	device_interface_desc(void *device_private_data)	throw();
		virtual	ssize_t	write_data(void *pdata,
						const ByteArray &data, int timeout)					throw();
		virtual	ssize_t read_data(void *device_private_data,
						ByteArray &data, size_t length, int timeout)		throw();
		virtual	void	release_device(
						void **device_private_data)							throw();
};

#endif
