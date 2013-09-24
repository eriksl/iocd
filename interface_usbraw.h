#ifndef _interface_usbraw_h_
#define _interface_usbraw_h_

#include <string>
using std::string;

#include "interface.h"
#include "if_private_data.h"

#include <stdint.h>
#include <libusb-1.0/libusb.h>

class Interfaces;
class InterfaceUSBraw;

class InterfaceUSBrawPrivateData : public InterfacePrivateData
{
	friend	InterfaceUSBraw;

	public:
				InterfaceUSBrawPrivateData()	throw();
		virtual	~InterfaceUSBrawPrivateData()	throw();

	private:

		libusb_device				*device;
		libusb_device_descriptor	*descriptor;
		libusb_device_handle		*handle;
		uint8_t						write_endpoint;
		uint8_t						read_endpoint;
};

class InterfaceUSBraw : public Interface
{
	friend class Interfaces;

	public:

				InterfaceUSBraw(Interfaces *root, ID id)	throw(exception);
		virtual	~InterfaceUSBraw()							throw();

		virtual	string name_short()		const	throw();
		virtual	string name_long()		const	throw();
		virtual	string interface_id()	const	throw();

	private:

		template<class DeviceT> void probe_single_device(
				uint8_t write_endpoint, uint8_t read_endpoint,
				int vendor, int product, int version = -1)					throw();

		virtual	void	probe_all_devices()									throw(exception);
		virtual	string	device_interface_desc(const InterfacePrivateData &)	throw();
		virtual	ssize_t	write_data(const InterfacePrivateData &,
						const ByteArray &data, int timeout)					throw();
		virtual	ssize_t read_data(const InterfacePrivateData &,
						ByteArray &data, size_t length, int timeout)		throw();
};

#endif
