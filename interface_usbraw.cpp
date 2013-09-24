#include "interface_usbraw.h"
#include "devices.h"
#include "device_k8055.h"
#include "device_atmel.h"
#include "cppstreams.h"
#include "util.h"

#include <typeinfo>
using std::bad_cast;

InterfaceUSBrawPrivateData::InterfaceUSBrawPrivateData() throw()
{
}

InterfaceUSBrawPrivateData::~InterfaceUSBrawPrivateData() throw()
{
	if(device)
	{
		//lock(); // FIXME
		libusb_unref_device(device);
		device = 0;
		//unlock(); // FIXME
	}

	Util::dlog("&& if_usbrawdata destructor\n");

	if(descriptor)
	{
		Util::dlog("&& if_usbrawdata destructor delete descriptor\n");
		delete descriptor;
		descriptor = 0;
	}

	if(handle)
	{
		//lock(); FIXME
		libusb_release_interface(handle, 0);
		libusb_attach_kernel_driver(handle, 0);
		libusb_close(handle);
		handle = 0;
		//unlock(); FIXME
	}
}

InterfaceUSBraw::InterfaceUSBraw(Interfaces *root_in, ID id_in) throw(exception)
	:	Interface(root_in, id_in)
{
	ssize_t rv;

	if((rv = libusb_init(0)) != 0)
		throw(minor_exception(Util::usb_error_string(rv)));

	libusb_set_debug(0, 3);
}

InterfaceUSBraw::~InterfaceUSBraw() throw()
{
	devices.clear();
	lock();
	libusb_exit(0);
	unlock();
}

string InterfaceUSBraw::name_short() const throw()
{
	return("usbraw");
}

string InterfaceUSBraw::name_long() const throw()
{
	return("USB raw interface");
}

string InterfaceUSBraw::device_interface_desc(const InterfacePrivateData &pd) throw()
{
	ostringstream rv;
	int bus, address;

	try
	{
		const InterfaceUSBrawPrivateData &urpd = dynamic_cast<const InterfaceUSBrawPrivateData &>(pd);
		lock();
		bus		= libusb_get_bus_number(urpd.device);
		address = libusb_get_device_address(urpd.device);
		unlock();
    	rv << bus << ":" << address;
    	return(rv.str());
	}
	catch(bad_cast)
	{
		Util::dlog("if_usbraw::device_interface_desc: pd == null\n");
	}
	catch(...)
	{
		Util::dlog("if_usbraw::device_interface_desc: unknown exception\n");
	}

	return("<unknown>");
}

string InterfaceUSBraw::interface_id() const throw()
{
	return(name_short());
}

void InterfaceUSBraw::probe_all_devices() throw(exception)
{
	probe_single_device<DeviceK8055>(0x01, 0x81, 0x10cf, 0x5500);
	probe_single_device<DeviceK8055>(0x01, 0x81, 0x10cf, 0x5502);
	probe_single_device<DeviceAtmel>(0x00, 0x00, 0x16c0, 0x05dc);
}

template<class DeviceT> void InterfaceUSBraw::probe_single_device(
			uint8_t write_endpoint, uint8_t read_endpoint,
			int match_vendor, int match_product, int match_version) throw()
{
	int							ix;
	ssize_t						rv;
	libusb_device				**device_list = 0;
	libusb_device				*usbdev;
	libusb_device_descriptor	*descriptor = 0;
	libusb_device_handle		*handle = 0;
	DeviceT     				*device = 0;
    string      				error;
	InterfaceUSBrawPrivateData	*urpd = 0;

	Util::dlog("DD InterfaceUSBraw::probe_single_device: %s:%d/%d:0x%04x/0x%04x/0x%04x\n",
			DeviceT::name_short_static().c_str(),
			write_endpoint, read_endpoint, match_vendor, match_product, match_version);

    try
    {
		descriptor = new libusb_device_descriptor;

		lock();
		rv = libusb_get_device_list(0, &device_list);
		unlock();

		if(rv < 0)
			throw(minor_exception(string("II if_usbraw: ") + Util::usb_error_string(rv)));

        Util::vlog("II if_usbraw: probing for %s@0x%04x:0x%04x\n",
				DeviceT::name_short_static().c_str(), match_vendor, match_product);

		for(ix = 0; (usbdev = device_list[ix]); ix++)
		{
			lock();
			rv = libusb_get_device_descriptor(usbdev, descriptor);
			unlock();

			if(rv < 0)
			{
				Util::vlog("II if_usbraw: device 0x%04x:0x%04x: %s: cannot read descriptor\n",
						descriptor->idVendor, descriptor->idProduct, Util::usb_error_string(rv).c_str());
				continue;
			}

			if((descriptor->idVendor != match_vendor) || (descriptor->idProduct != match_product))
				continue;

			if((match_version == -1) || (match_version == descriptor->bcdDevice))
				break;
		}

		if(!usbdev)
			throw(minor_exception("device not found"));

		lock();
		libusb_ref_device(usbdev);
		unlock();

		urpd = new InterfaceUSBrawPrivateData;
		urpd->device			= usbdev;
		urpd->descriptor		= descriptor;
		urpd->handle			= 0;
		urpd->write_endpoint	= write_endpoint;
		urpd->read_endpoint		= read_endpoint;

		descriptor = 0;	// avoid pointer aliasing

		lock();
		rv = libusb_open(urpd->device, &handle);
		unlock();

		if(rv < 0)
			throw(minor_exception(string("if_usbraw: libusb_open: ") + Util::usb_error_string(rv)));

		urpd->handle = handle;

		lock();
		rv = libusb_detach_kernel_driver(handle, 0);
		unlock();

		if(rv == 0)
			Util::dlog("DD if_usbraw: libusb_detach_kernel_driver: OK\n");
		else
			Util::dlog("DD if_usbraw: libusb_detach_kernel_driver: warning: %s\n",
					Util::usb_error_string(rv).c_str());

		lock();
		rv = libusb_set_configuration(handle, 1);
		unlock();

		if(rv != 0)
			throw(minor_exception(string("if_usbraw: libusb_set_configuration: ") + Util::usb_error_string(rv)));

		lock();
		rv = libusb_claim_interface(handle, 0);
		unlock();

		if(rv != 0)
			throw(minor_exception(string("if_usbraw: libusb_claim_interface: ") + Util::usb_error_string(rv)));

        device = new DeviceT(root, ID(id.interface, enumerator++), urpd);

        Util::vlog("II if_usbraw: probe for %s successful\n", device->name_short().c_str());
        devices.add(device);
        device->find_controls();
    }
    catch(iocd_exception e)
    {
		Util::dlog("if_usbraw::probe: catch %s\n", e.message.c_str());

		if(device)
		{
			delete device;
			device = 0;
		}
        error = e.message;
    }
    catch(...)
    {
		Util::dlog("if_usbraw::probe: catch ...\n");

		if(device)
		{
			delete device;
			device = 0;
		}
        error = "<unspecified error";
    }

	if(device_list)
	{
		lock();
		libusb_free_device_list(device_list, 1);
		unlock();
		device_list = 0;
	}

	if(!device)
	{
		delete descriptor;
		Util::vlog("II if_usbraw: probe for %s@%04x:%04x unsuccessful: %s\n", DeviceT::name_short_static().c_str(), match_product, match_vendor, error.c_str());
	}
}

ssize_t InterfaceUSBraw::write_data(const InterfacePrivateData &pd, const ByteArray &byte_array, int timeout) throw()
{
	uint8_t	*data = 0;
	size_t	length;
	int		transferred = 0;
	ssize_t	rv;

	try
	{
		const InterfaceUSBrawPrivateData &urpd = dynamic_cast<const InterfaceUSBrawPrivateData &>(pd);

		Util::dlog("DD if_usbraw: write_data, write_endpoint = %02x\n", urpd.write_endpoint);

		data = byte_array.to_memory(&length);

		lock();

		if(urpd.write_endpoint == 0)
		{
			rv = libusb_control_transfer(urpd.handle,
				LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_ENDPOINT,
				0, 0, 0, 
				data, length, timeout);

			transferred = length;
		}
		else
		{
			rv = libusb_interrupt_transfer(urpd.handle,
				urpd.write_endpoint, data, length, &transferred, timeout);
		}

		unlock();

		if((rv < 0) || (transferred != (int)length))
		{
			Util::vlog("II if_usbraw.write_data: %s\n", Util::usb_error_string(rv).c_str());
			transferred = 0;
		}
	}
	catch(bad_cast)
	{
		Util::dlog("if_usbraw::write_data: pd == null\n");
	}
	catch(...)
	{
		Util::dlog("if_usbraw::write_data: unknown exception\n");
	}

	delete [] data;
	return(transferred);
}

ssize_t InterfaceUSBraw::read_data(const InterfacePrivateData &pd, ByteArray &byte_array, size_t length_in, int timeout) throw()
{
	uint8_t	*data;
	ssize_t rv;
	int transferred = 0;
	size_t length;

	if(length_in == 0) // unknown length
		length = 254;
	else
		length = length_in;

	data = new uint8_t[length];

	try
	{
		const InterfaceUSBrawPrivateData &urpd = dynamic_cast<const InterfaceUSBrawPrivateData &>(pd);

		Util::dlog("DD if_usbraw: read_data, read_endpoint = %02x\n", urpd.read_endpoint);

		lock();

		if(urpd.read_endpoint == 0)
		{
			rv = libusb_control_transfer(urpd.handle,
				LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_ENDPOINT,
				0, 0, 0,
				data, length, timeout);
	
			transferred = rv;
		}
		else
		{
			rv = libusb_interrupt_transfer(urpd.handle,
				urpd.read_endpoint, data, length, &transferred, timeout);
		}

		unlock();

		Util::dlog("DD if_usbraw.read_data: length = %d, read %d bytes\n", length, transferred);

		if(rv < 0)
		{
			Util::vlog("II if_usbraw.read_data: %s\n", Util::usb_error_string(rv).c_str());
			transferred = 0;
		}

		if((length_in == 0) && (transferred > 0) && (data[0] > 0))
		{
			Util::dlog("DD if_usbraw.read_data: zero length requested, data[0] = %d\n", data[0]);
			transferred = data[0];
			byte_array.from_memory(transferred, data + 1);
			Util::dlog("DD if_usbraw.read_data: zero length requested, transferred = %d\n", transferred);
		}
		else
		{
			byte_array.from_memory(transferred, data);
			Util::dlog("DD if_usbraw.read_data: nonzero length requested, transferred = %d\n", transferred);
		}

	}
	catch(bad_cast)
	{
		Util::dlog("if_usbraw::read_data: pd == null\n");
	}
	catch(...)
	{
		Util::dlog("if_usbraw::read_data: unknown exception\n");
	}

	delete [] data;
	return(transferred);
}
