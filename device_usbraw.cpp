#include "device_usbraw.h"
#include "interface_usbraw.h"
#include "cppstreams.h"
#include "util.h"

DeviceUSBraw::DeviceUSBraw(Interfaces *root_in, ID id_in,
		libusb_device *device_in, int write_ep_in, int read_ep_in) throw(exception)
	:	Device(root_in, id_in),
	read_endpoint(read_ep_in),
	write_endpoint(write_ep_in),
	device(device_in),
	handle(0)
{
	ssize_t						rv;
	libusb_device_descriptor	desc;

	bus		= libusb_get_bus_number(device);
	address	= libusb_get_device_address(device);

	libusb_ref_device(device);

	if((rv = libusb_get_device_descriptor(device, &desc)) != 0)
		throw(minor_exception(string("DD usbraw: libusb_get_device_descriptor: ") + libusb_error_name(rv)));

	vendor	= desc.idVendor;
	product = desc.idProduct;

	if((rv = libusb_open(device, &handle)) != 0)
		throw(minor_exception(string("DD usbraw: libusb_open: ") + libusb_error_name(rv)));

	if((rv = libusb_detach_kernel_driver(handle, 0)) == 0)
		Util::dlog("DD usbraw: libusb_detach_kernel_driver: OK\n");
	else
		Util::dlog("DD usbraw: libusb_detach_kernel_driver: %s\n", libusb_error_name(rv));

	if((rv = libusb_set_configuration(handle, 1)) != 0)
		throw(minor_exception(string("DD usbraw: libusb_set_configuration: ") + libusb_error_name(rv)));

	if((rv = libusb_claim_interface(handle, 0)) != 0)
		throw(minor_exception(string("DD usbraw: libusb_claim_interface: ") + libusb_error_name(rv)));
}

DeviceUSBraw::~DeviceUSBraw() throw()
{
	ssize_t rv;

	if(handle)
	{
		if((rv = libusb_release_interface(handle, 0)) == 0)
			Util::dlog("DD usbraw: libusb_release_interface: OK\n");
		else
			Util::dlog("DD usbraw: libusb_release_interface: %s\n", libusb_error_name(rv));

		if((rv = libusb_attach_kernel_driver(handle, 0)) == 0)
			Util::dlog("DD usbraw: libusb_attach_kernel_driver: OK\n");
		else
			Util::dlog("DD usbraw: libusb_attach_kernel_driver: %s\n", libusb_error_name(rv));

		libusb_close(handle);
	}

	libusb_unref_device(device);
}

string DeviceUSBraw::device_id() const throw()
{
	ostringstream rv;
	rv << name_short() << "@" << bus << "," << address;
	return(rv.str());
}

ssize_t DeviceUSBraw::send_command(ssize_t length, uint8_t *data, int timeout) throw()
{
	ssize_t sent = 0;

	try
	{
		InterfaceUSBraw::cmd_t cmd;

		cmd.endpoint	= write_endpoint;
		cmd.handle		= handle;
		cmd.timeout		= timeout;
		cmd.length		= int(length);
		cmd.data		= data;

		Device::command(&cmd);

		sent			= cmd.transferred;
	}
	catch(minor_exception e)
	{
		Util::vlog("DD send_command: %s\n", e.message.c_str());
	}

	return(sent);
}

ssize_t DeviceUSBraw::receive_command(ssize_t length, uint8_t *data, int timeout) throw()
{
	ssize_t received = 0;

	try
	{
		InterfaceUSBraw::cmd_t cmd;

		cmd.endpoint	= read_endpoint;
		cmd.handle		= handle;
		cmd.timeout		= timeout;
		cmd.length		= int(length);
		cmd.data		= data;

		Device::command(&cmd);

		received		= cmd.transferred;
	}
	catch(minor_exception e)
	{
		Util::vlog("DD receive_command: %s\n", e.message.c_str());
	}

	return(received);
}

