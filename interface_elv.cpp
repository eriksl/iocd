#include "interface_elv.h"
#include "device_atmel.h"
#include "device_tmp275.h"
#include "device_digipicco.h"
#include "device_tsl2550.h"
#include "device_ds1731.h"
#include "devices.h"
#include "cppstreams.h"
#include "util.h"

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>

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
	return("elv_i2c");
}

string InterfaceELV::name_long_static() throw()
{
	return("ELV I2C interface");
}

string InterfaceELV::name_short() const throw()
{
	return(name_short_static());
}

string InterfaceELV::name_long() const throw()
{
	return(name_long_static());
}

string InterfaceELV::interface_id() const throw()
{
	return(name_short() + ":" + device_node);
}

void InterfaceELV::open(string open_device) throw(exception)
{
	int				result;
	struct termios	tio;

	if((fd = ::open(open_device.c_str(), O_RDWR | O_NOCTTY | O_EXCL, 0)) < 0)
		throw(minor_exception(string("InterfaceELV::open: cannot open device ") + open_device));

	ioctl(fd, TIOCEXCL, 1);

	if(ioctl(fd, TIOCMGET, &result))
	{
		::close(fd);
		throw(minor_exception("InterfaceELV::open: error in TIOCMGET"));
	}

	result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(fd, TIOCMSET, &result))
	{
		::close(fd);
		throw(minor_exception("InterfaceELV::open: error in TIOCMSET"));
	}

	result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

	if(ioctl(fd, TIOCMSET, &result))
	{
		::close(fd);
		throw(minor_exception("InterfaceELV::open: error in TIOCMSET"));
	}

	if(tcgetattr(fd, &tio) == 1)
	{
		::close(fd);
		throw(minor_exception("InterfaceELV::open: error in tcgetattr"));
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
		throw(minor_exception("InterfaceELV::open: error in tcsetattr"));
	}

	string rv;

	try
	{
		rv = command("z 4b", 5000, 2);
	}
	catch(minor_exception e)
	{
		Util::dlog("open: minor exception during reset: %s\n", e.message.c_str());
		::close(fd);
		throw(minor_exception("Interface::ELV: not found"));
	}
	catch(major_exception e)
	{
		Util::dlog("open: major exception during reset: %s\n", e.message.c_str());
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

string InterfaceELV::interface_command(const string &cmd_in, int timeout, int chunks) throw(exception)
{
	static char		buffer[256];
	string			cmd;
	struct pollfd	pfd;
	ssize_t			len;
	int				pr;
	string			rv;
	struct timespec	start, now;
	int				timeout_left;

	if(clock_gettime(CLOCK_MONOTONIC, &start))
		throw(minor_exception("InterfaceELV::interface_command: clock_gettime error\n"));

	cmd = string(":") + cmd_in + string("\n");

	Util::dlog(">> %s\n", Util::remove_newlines(cmd).c_str());

	while(true)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(minor_exception("InterfaceELV::interface_command: clock_gettime error\n"));

		timeout_left = Util::timespec_diff(start, now);

		if((timeout > 0) && (timeout_left < 0))
			throw(minor_exception("InterfaceELV::interface_command: fatal timeout (1)"));

		pfd.fd		= fd;
		pfd.events	= POLLIN | POLLERR;

		pr = poll(&pfd, 1, 0);

		if(pr < 0)
			throw(major_exception("InterfaceELV::interface_command: poll error (1)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(major_exception("InterfaceELV::interface_command: poll error (2)"));

		if(pfd.revents & POLLIN)
		{
			len = ::read(fd, buffer, sizeof(buffer) - 1);
			buffer[len] = 0;
			Util::dlog("clearing backlog, cleared %d bytes: %s\n", len, buffer);
		}
	}

	if(clock_gettime(CLOCK_MONOTONIC, &now))
		throw(minor_exception("InterfaceELV::interface_command: clock_gettime error\n"));

	timeout_left = timeout > 0 ? timeout - Util::timespec_diff(start, now) : -1;

	pfd.fd		= fd;
	pfd.events	= POLLOUT | POLLERR;

	pr = poll(&pfd, 1, timeout_left);

	if(pr < 0)
		throw(major_exception("InterfaceELV::interface_command: poll error (3)"));

	if(pfd.revents & POLLERR)
		throw(major_exception("InterfaceELV::interface_command: poll error (4)"));

	if(!(pfd.revents & POLLOUT))
		throw(minor_exception("InterfaceELV::interface_command: fatal timeout (2)"));

	if(::write(fd, cmd.c_str(), cmd.length()) != (ssize_t)cmd.length())
		throw(major_exception("InterfaceELV::interface_command: write error"));

	while(chunks > 0)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(minor_exception("InterfaceELV::interface_command: clock_gettime error\n"));

		timeout_left = timeout > 0 ? timeout - Util::timespec_diff(start, now) : -1;

		if((timeout > 0) && (timeout_left < 0))
			break;

		pfd.fd		= fd;
		pfd.events	= POLLIN | POLLERR;

		//Util::dlog("read poll, timeout = %d\n", timeout_left);

		pr = poll(&pfd, 1, timeout_left);

		if(pr < 0)
			throw(major_exception("InterfaceELV::interface_command: poll error (5)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(major_exception("InterfaceELV::interface_command: poll error (6)"));

		if(pfd.revents & POLLIN)
		{
			len = ::read(fd, buffer, sizeof(buffer) - 1);
			buffer[len] = 0;
			//Util::dlog("received %d bytes: %s\n", len, buffer);
			rv += buffer;
			chunks--;
		}
	}

	Util::dlog("<< %s\n", Util::remove_newlines(rv).c_str());

	return(rv);
}

void InterfaceELV::find_devices() throw()
{
	probe_device<DeviceAtmel>(0x02);
	probe_device<DeviceAtmel>(0x03);
	probe_device<DeviceTMP275>(0x49);
	probe_device<DeviceDigipicco>(0x78);
	probe_device<DeviceTSL2550>(0x39);
	probe_device<DeviceDS1731>(0x48);
}

template<class DeviceT> void InterfaceELV::probe_device(int address) throw()
{
	DeviceT		*device = 0;
	string 		error;

	try
	{
		Util::dlog("DD probing for %s:0x%02x\n", DeviceT::name_short_static().c_str(), address);
		device = new DeviceT(root, ID(id.interface, enumerator++), address);
	}
	catch(minor_exception e)
	{
		error = e.message;
	}
	catch(...)
	{
		error = "<unspecified error";
	}

	if(device)
	{
		Util::dlog("DD probe for %s successful\n", device->device_id().c_str());
		devices.add(device);
		Util::dlog("DD finding controls for %s\n", device->device_id().c_str());
		device->find_controls();
		Util::dlog("DD finding controls for %s successful\n", device->device_id().c_str());
	}
	else
		Util::dlog("DD probe for %s at 0x%02x unsuccessful: %s\n", DeviceT::name_short_static().c_str(), address, error.c_str());
}
