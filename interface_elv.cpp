#include "interface_elv.h"
#include "device_tmp275.h"
//#include "device_digipicco.h"
//#include "device_tsl2550.h"
//#include "device_ds1731.h"
#include "devices.h"
#include "cppstreams.h"
#include "util.h"

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

InterfaceELV::InterfaceELV(Interfaces *root_in, ID id_in, string device_node_in) throw(exception)
	:	Interface(root_in, id_in)
{
	size_t	pos;

	pos = device_node_in.find_last_of("/");

	if(pos != string::npos)
		device_node = device_node_in.substr(pos + 1);
	else
		device_node = device_node_in;

	open(device_node_in);
}

InterfaceELV::~InterfaceELV() throw()
{
}

string InterfaceELV::name_short_static() throw()
{
	return("elvusbi2c");
}

string InterfaceELV::name_long_static() throw()
{
	return("ELV USB to I2C converter");
}

string InterfaceELV::device_interface_desc(void *pdata_void) throw()
{
	if_usbelv_pdata_t *pdata = (if_usbelv_pdata_t *)pdata_void;

	if(!pdata)
		return("<unknown>");

    return(string("0x") + Util::hex_to_string(pdata->address, 2));
}

string InterfaceELV::name_short() throw()
{
	return(name_short_static());
}

string InterfaceELV::name_long() throw()
{
	return(name_long_static());
}

string InterfaceELV::interface_id() throw()
{
	return(name_short() + "@" + device_node);
}

void InterfaceELV::open(string open_device) throw(exception)
{
	int				result;
	struct termios	tio;

	if((fd = ::open(open_device.c_str(), O_RDWR | O_NOCTTY | O_EXCL, 0)) < 0)
		throw(minor_exception(string("if_usbelv::open: cannot open device ") + open_device));

	ioctl(fd, TIOCEXCL, 1);

	if(ioctl(fd, TIOCMGET, &result))
	{
		::close(fd);
		throw(minor_exception("if_usbelv::open: error in TIOCMGET"));
	}

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(fd, TIOCMSET, &result))
	{
		::close(fd);
		throw(minor_exception("if_usbelv::open: error in TIOCMSET"));
	}

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(fd, TIOCMSET, &result))
	{
		::close(fd);
		throw(minor_exception("if_usbelv::open: error in TIOCMSET"));
	}

	if(tcgetattr(fd, &tio) == 1)
	{
		::close(fd);
		throw(minor_exception("if_usbelv::open: error in tcgetattr"));
	}

	tio.c_iflag &= ~(BRKINT | INPCK | INLCR | IGNCR | IUCLC |
					IXON | IXOFF | IXANY | IMAXBEL | ISTRIP | ICRNL);
	tio.c_iflag |=	(IGNBRK | IGNPAR);

	tio.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONOCR | ONLRET | OFILL | ONLCR);
	tio.c_oflag |= 0;

	tio.c_cflag &=	~(CSIZE | PARENB | PARODD | HUPCL | CRTSCTS);
	tio.c_cflag |= (CREAD | CS8 | CSTOPB | CLOCAL);

	tio.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO | ECHOE | ECHOK |
					ECHONL | ECHOCTL | ECHOPRT | ECHOKE | FLUSHO | TOSTOP |
					PENDIN | IEXTEN | NOFLSH);
	tio.c_lflag |=	0;

	cfsetispeed(&tio, B115200);
	cfsetospeed(&tio, B115200);

	if(tcsetattr(fd, TCSANOW, &tio) == 1)
	{
		::close(fd);
		throw(minor_exception("if_usbelv::open: error in tcsetattr"));
	}

	string rv;
	uint8_t buffer[256];
	size_t length;

	try
	{
		static const uint8_t *resetstring = (const uint8_t *)":z 4b\n";

		write_raw(strlen((const char *)resetstring), resetstring, 2000);

		length = sizeof(buffer);
		buffer[length] = '\0';
		read_raw(&length, buffer - 1, 1000);
		rv = (const char *)buffer;

		length = sizeof(buffer);
		read_raw(&length, buffer - 1, 3000);
		buffer[length] = '\0';
		rv += (const char *)buffer;
	}
	catch(minor_exception e)
	{
		Util::dlog("if_usbelv:open: minor exception during reset: %s\n", e.message.c_str());
		::close(fd);
		throw(minor_exception("Interface::ELV: not found"));
	}
	catch(major_exception e)
	{
		Util::dlog("if_usbelv:open: major exception during reset: %s\n", e.message.c_str());
		::close(fd);
		throw(minor_exception("Interface::ELV: not found"));
	}

	if(rv.find("ELV USB-I2C-Interface v1.6") == string::npos)
	{
		Util::dlog("InterfaceELV: id string not recognised\n");
		::close(fd);
		throw(minor_exception("interface ELV not found"));
	}
}

void InterfaceELV::probe_all_devices() throw()
{
	//probe_single_device<DeviceAtmel>(0x02); // FIXME
	//probe_single_device<DeviceAtmel>(0x03); // FIXME
	probe_single_device<DeviceTMP275>(0x49); // FIXME
	//probe_single_device<DeviceDigipicco>(0x78); // FIXME
	//probe_single_device<DeviceTSL2550>(0x39); // FIXME
	//probe_single_device<DeviceDS1731>(0x48); // FIXME
}

template<class DeviceT> void InterfaceELV::probe_single_device(int address) throw()
{
	DeviceT				*device = 0;
	string 				error;
	if_usbelv_pdata_t	*pdata;

	Util::dlog("DD if_elvusb: probe_single_device: %s:0x%02x\n", DeviceT::name_short_static().c_str(), address);

	pdata = new if_usbelv_pdata_t;
	pdata->address = address;

	try
	{
		device = new DeviceT(root, ID(id.interface, enumerator++), pdata);

		Util::dlog("DD if_elvusb: probe for %s successful\n", device->name_short().c_str());
		devices.add(device);
		device->find_controls();
	}
	catch(iocd_exception e)
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
		Util::dlog("DD probe for %s at 0x%02x unsuccessful: %s\n", DeviceT::name_short_static().c_str(), address, error.c_str());
}

void InterfaceELV::write_raw(size_t length, const uint8_t *data, int timeout) throw(exception)
{
	struct pollfd	pfd;
	uint8_t			buffer[256];
	int				pr;
	int				tmplength;

	Util::dlog("DD if_elvusb: write_raw: [%d] \"", length);
	size_t ix;
	for(ix = 0; ix < length; ix++)
	{
		if(data[ix] >= ' ' && data[ix] < '~')
			Util::dlog("%c", data[ix]);
		else
			Util::dlog("[%02x]", data[ix]);
	}
	Util::dlog("\"\n");


	pfd.fd		= fd;
	pfd.events	= POLLIN | POLLERR;

	while(true)
	{
		pr = poll(&pfd, 1, 0);

		if(pr < 0)
			throw(major_exception("InterfaceELV::write_raw: poll error (1)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(major_exception("InterfaceELV::write_raw: poll error (2)"));

		if(pfd.revents & POLLIN)
		{
			tmplength = ::read(fd, buffer, sizeof(buffer) - 1);
			buffer[tmplength] = 0;
			Util::dlog("clearing backlog, cleared %d bytes: %s\n", tmplength, buffer);
		}
	}

	pfd.events	= POLLOUT | POLLERR;

	pr = poll(&pfd, 1, timeout);

	if(pr < 0)
		throw(major_exception("InterfaceELV::write_raw: poll error (3)"));

	if(pfd.revents & POLLERR)
		throw(major_exception("InterfaceELV::write_raw: poll error (4)"));

	if(!(pfd.revents & POLLOUT))
		throw(minor_exception("InterfaceELV::write_raw: timeout"));

	if(::write(fd, data, length) != (ssize_t)length)
		throw(major_exception("InterfaceELV::write_raw: write error"));
}

void InterfaceELV::read_raw(size_t *length, uint8_t *data, int timeout) throw(exception)
{
	struct pollfd	pfd;
	int				pr;

	pfd.fd		= fd;
	pfd.events	= POLLIN | POLLERR;

	pr = poll(&pfd, 1, timeout);

	if(pr != 1)
		throw(major_exception("InterfaceELV::read_raw: poll error (1)"));

	if(pfd.revents & POLLERR)
		throw(major_exception("InterfaceELV::read_raw: poll error (2)"));

	if(pfd.revents & POLLIN)
		*length = ::read(fd, data, *length);
	else
		throw(major_exception("InterfaceELV::read_raw: timeout"));

	Util::dlog("DD if_elvusb: read_raw: [%d]\"", *length);
	size_t ix;
	for(ix = 0; ix < *length; ix++)
	{
		if(data[ix] >= ' ' && data[ix] < '~')
			Util::dlog("%c", data[ix]);
		else
			Util::dlog("[%02x]", data[ix]);
	}
	Util::dlog("\"\n");
}

ssize_t	InterfaceELV::write_data(void *pdata_void, const ByteArray &data_in, int timeout) throw(exception)
{
	stringstream				in;
	string						out;
	if_usbelv_pdata_t			*pdata = (if_usbelv_pdata_t *)pdata_void;
	ByteArray::const_iterator	it;

	in << ":s " << hex << setw(2) << setfill('0') << (pdata->address << 1) << " w";

	for(it = data_in.begin(); it != data_in.end(); it++)
		in << " " << hex << setw(2) << setfill('0') << (int)*it;

	in << " p\n";

	out = in.str();

	write_raw(out.length(), (const uint8_t *)out.c_str(), timeout);

	return(data_in.size());
}

ssize_t InterfaceELV::read_data(void *pdata_void, ByteArray &bytes, size_t length, int timeout) throw(exception)
{
	stringstream				out;
	string						in;
	if_usbelv_pdata_t			*pdata = (if_usbelv_pdata_t *)pdata_void;
	uint8_t						data[1024];

	out << ":s " << hex << setw(2) << setfill('0') << (pdata->address << 1);
	out << " r " << hex << setw(2) << setfill('0') << length << " p\n";

	write_raw(out.str().length(), (const uint8_t *)out.str().c_str(), timeout);

	length = sizeof(data);
	read_raw(&length, data, timeout);

	in.assign((const char *)data, length);

	length = Util::parse_bytes(in, bytes);

	Util::dlog("DD read_data: return: %s\n", ((string)bytes).c_str());

	return(length);
}

void InterfaceELV::release_device(void **pdata_void) throw()
{
	if_usbelv_pdata_t **pdata;

	pdata = (if_usbelv_pdata_t **)pdata_void;

	if(!pdata || !*pdata)
		return;

	delete *pdata;
	*pdata = 0;
}
