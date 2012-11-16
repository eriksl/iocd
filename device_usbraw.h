#ifndef _device_usbraw_h_
#define _device_usbraw_h_

#include "device.h"
#include "id.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

#include <libusb.h>

class Interfaces;

class DeviceUSBraw : public Device
{
	public:

				DeviceUSBraw(Interfaces *root, ID, libusb_device *device,
						int write_endpoint, int read_endpoint)	throw(exception);
		virtual	~DeviceUSBraw()									throw();

				string	device_id()						const	throw();

	protected:

		int						product;
		int						vendor;
		int						bus;
		int						address;
		int						read_endpoint;
		int						write_endpoint;
		libusb_device			*device;
		libusb_device_handle	*handle;

		ssize_t	send_command(ssize_t length, uint8_t *data, int timeout = 1000) throw();
		ssize_t receive_command(ssize_t length, uint8_t *data, int timeout = 1000) throw();

	private:

		virtual	string	name_short()			const	throw() = 0;
		virtual	string	name_long()				const	throw() = 0;
};
#endif
