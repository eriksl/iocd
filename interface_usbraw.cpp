#include "interface_usbraw.h"
#include "devices.h"
//#include "device_k8055.h"
#include "device_atmel.h"
#include "cppstreams.h"
#include "util.h"

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

string InterfaceUSBraw::name_short_static() throw()
{
	return("usbraw");
}

string InterfaceUSBraw::name_long_static() throw()
{
	return("USB raw interface");
}

string InterfaceUSBraw::device_interface_desc(void *pdata_void) throw()
{
	ostringstream rv;
	if_usbraw_pdata_t *pdata = (if_usbraw_pdata_t *)pdata_void;
	int bus, address;

	if(!pdata)
		return("<unknown>");

	lock();
	bus		= libusb_get_bus_number(pdata->device);
	address = libusb_get_device_address(pdata->device);
	unlock();
    rv << name_short() << "@" << bus << ":" << address;

    return(rv.str());
}

string InterfaceUSBraw::name_short() throw()
{
	return(name_short_static());
}

string InterfaceUSBraw::name_long() throw()
{
	return(name_long_static());
}

string InterfaceUSBraw::interface_id() throw()
{
	return(name_short());
}

void InterfaceUSBraw::probe_all_devices() throw(exception)
{
	//probe_device<DeviceK8055>(0x10cf, 0x5500);
	//probe_device<DeviceK8055>(0x10cf, 0x5502);
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
	if_usbraw_pdata_t			*pdata = 0;
	DeviceT     				*device = 0;
    string      				error;

	Util::dlog("DD InterfaceUSBraw::probe_single_device: %d/%d 0x%04x/0x%04x/0x%04x\n",
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

		pdata = new if_usbraw_pdata_t;
		pdata->device			= usbdev;
		pdata->descriptor		= descriptor;
		pdata->handle			= 0;
		pdata->write_endpoint	= write_endpoint;
		pdata->read_endpoint	= read_endpoint;

		lock();
		rv = libusb_open(pdata->device, &handle);
		unlock();

		if(rv < 0)
		{
			release_device((void **)&pdata);
			throw(minor_exception(string("if_usbraw: libusb_open: ") + Util::usb_error_string(rv)));
		}

		pdata->handle = handle;

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
		{
			release_device((void **)&pdata);
			throw(minor_exception(string("if_usbraw: libusb_set_configuration: ") + Util::usb_error_string(rv)));
		}

		lock();
		rv = libusb_claim_interface(handle, 0);
		unlock();

		if(rv != 0)
		{
			release_device((void **)&pdata);
			throw(minor_exception(string("if_usbraw: libusb_claim_interface: ") + Util::usb_error_string(rv)));
		}

        device = new DeviceT(root, ID(id.interface, enumerator++), pdata);

        Util::vlog("II if_usbraw: probe for %s successful\n", device->device_id().c_str());
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

	if(device_list)
	{
		lock();
		libusb_free_device_list(device_list, 1);
		unlock();
		device_list = 0;
	}

	if(!device)
		Util::vlog("II if_usbraw: probe for %s@%04x:%04x unsuccessful: %s\n", DeviceT::name_short_static().c_str(), match_product, match_vendor, error.c_str());
}

void InterfaceUSBraw::release_device(void **pdata_void) throw()
{
	if_usbraw_pdata_t **pdata;

	pdata = (if_usbraw_pdata_t **)pdata_void;

	if(!pdata || !*pdata)
		return;

	if((**pdata).device)
	{
		lock();
		libusb_unref_device((**pdata).device);
		unlock();
		(**pdata).device = 0;
	}

	if((**pdata).descriptor)
	{
		delete (**pdata).descriptor;
		(**pdata).descriptor = 0;
	}

	if((**pdata).handle)
	{
		lock();
		libusb_release_interface((**pdata).handle, 0);
		libusb_attach_kernel_driver((**pdata).handle, 0);
		libusb_close((**pdata).handle);
		unlock();
		(**pdata).handle = 0;
	}

	delete *pdata;
	*pdata = 0;
}

ssize_t InterfaceUSBraw::write_data(void *pdata_void, const ByteArray &byte_array, int timeout) throw()
{
	uint8_t	*data;
	size_t	length;
	int		transferred;
	ssize_t	rv;

	if_usbraw_pdata_t *pdata = (if_usbraw_pdata_t *)pdata_void;

	data = byte_array.to_memory(&length);

	lock();

	if(pdata->write_endpoint == 0)
	{
		rv = libusb_control_transfer(pdata->handle,
			LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_ENDPOINT,
			0, 0, 0, 
			data, length, timeout);

		transferred = length;
	}
	else
	{
		transferred = 0;

		rv = libusb_interrupt_transfer(pdata->handle,
			pdata->write_endpoint, data, length, &transferred, timeout);
	}

	unlock();

	if((rv < 0) || (transferred != (int)length))
	{
		Util::vlog("II if_usbraw.write_data: %s\n", Util::usb_error_string(rv).c_str());
		transferred = 0;
	}

	delete [] data;

	return(transferred);
}

ssize_t InterfaceUSBraw::read_data(void *pdata_void, ByteArray &byte_array, int timeout) throw()
{
	uint8_t	data[254];
	size_t	length = sizeof(data);
	ssize_t rv;
	int transferred;

	if_usbraw_pdata_t *pdata = (if_usbraw_pdata_t *)pdata_void;

	lock();

	if(pdata->read_endpoint == 0)
	{
		rv = libusb_control_transfer(pdata->handle,
			LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_ENDPOINT,
			0, 0, 0,
			data, length, timeout);

		transferred = rv;
	}
	else
	{
		transferred = 0;

		rv = libusb_interrupt_transfer(pdata->handle,
			pdata->read_endpoint, data, length, &transferred, timeout);
	}

	unlock();

	if(rv < 0)
	{
		Util::vlog("II if_usbraw.read_data: %s\n", Util::usb_error_string(rv).c_str());
		transferred = 0;
	}

	byte_array.from_memory(transferred, data);

	return(transferred);
}
