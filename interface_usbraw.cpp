#include "interface_usbraw.h"
#include "devices.h"
#include "device_k8055.h"
#include "cppstreams.h"
#include "util.h"

InterfaceUSBraw::InterfaceUSBraw(Interfaces *root_in, ID id_in) throw(exception)
	:	Interface(root_in, id_in), device_list(0)
{
	ssize_t rv;

	if((rv = libusb_init(0)) != 0)
		throw(minor_exception(Util::usb_error_string(rv)));

	libusb_set_debug(0, 3);

	if((rv = libusb_get_device_list(0, &device_list)) < 0)
		throw(minor_exception(string("II usbraw: ") + Util::usb_error_string(rv)));
}

InterfaceUSBraw::~InterfaceUSBraw() throw()
{
	devices.clear();

	if(device_list)
		libusb_free_device_list(device_list, 1);
	libusb_exit(0);
}

string InterfaceUSBraw::name_short_static() throw()
{
	return("usbraw");
}

string InterfaceUSBraw::name_long_static() throw()
{
	return("USB raw interface");
}

string InterfaceUSBraw::name_short() const throw()
{
	return(name_short_static());
}

string InterfaceUSBraw::name_long() const throw()
{
	return(name_long_static());
}

string InterfaceUSBraw::interface_id() const throw()
{
	return(name_short());
}

void InterfaceUSBraw::interface_command(void *cmdptr) throw(exception)
{
	ssize_t					rv;
	InterfaceUSBraw::cmd_t	*cmd = static_cast<InterfaceUSBraw::cmd_t *>(cmdptr);

	if((rv = libusb_interrupt_transfer(cmd->handle, cmd->endpoint,
					cmd->data, cmd->length, &cmd->transferred, cmd->timeout)) != 0)
		throw(minor_exception(string("II interrupt_transfer: ") + Util::usb_error_string(rv)));
}

void InterfaceUSBraw::find_devices() throw()
{
	probe_device<DeviceK8055>(0x10cf, 0x5500);
	probe_device<DeviceK8055>(0x10cf, 0x5502);
}

template<class DeviceT> void InterfaceUSBraw::probe_device(int vendor, int product) throw()
{
	int							ix;
	ssize_t						rv;
	libusb_device				*usbdev;
	libusb_device_descriptor	desc;
	DeviceT     				*device = 0;
    string      				error;

    try
    {
        Util::dlog("II usbraw: probing for %s@0x%04x:0x%04x\n", DeviceT::name_short_static().c_str(), vendor, product);

		for(ix = 0; (usbdev = device_list[ix]); ix++)
		{
			rv = libusb_get_device_descriptor(usbdev, &desc);

			if(rv >= 0)
			{
				if(desc.idVendor == vendor && desc.idProduct == product)
					break;
			}
			else
				Util::dlog("II usbraw: error device 0x%04x:0x%04x: %s\n", desc.idVendor, desc.idProduct, Util::usb_error_string(rv).c_str());
		}

		if(!usbdev)
			throw(minor_exception("device not found"));

        device = new DeviceT(root, ID(id.interface, enumerator++), usbdev);

        Util::dlog("II usbraw: probe for %s successful\n", device->device_id().c_str());
        devices.add(device);
        device->find_controls();
    }
    catch(minor_exception e)
    {
		if(device)
		{
			delete device;
			device = 0;
		}
        error = e.message;
    }
    catch(...)
    {
		if(device)
		{
			delete device;
			device = 0;
		}
        error = "<unspecified error";
    }

	if(!device)
		Util::dlog("II usbraw: probe for %s@%04x:%04x unsuccessful: %s\n", DeviceT::name_short_static().c_str(), product, vendor, error.c_str());
}
