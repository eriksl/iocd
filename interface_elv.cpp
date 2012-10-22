#include "interface_elv.h"
#include "device_atmel.h"
#include "device_tmp275.h"
#include "device_digipicco.h"
#include "device_tsl2550.h"
#include "device_ds1731.h"
#include "devices.h"
#include "syslog.h"

#include "cppstreams.h"

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>

InterfaceELV::InterfaceELV(Interfaces *interfaces_in,
			int generation_in, int parent_id_in, int ordinal_in,
			string parent_path_in, string path_in) throw(exception)
	:
		Interface(interfaces_in, generation_in, parent_id_in, ordinal_in, parent_path_in, path_in)
{
	string	device_short;
	size_t	pos;

	_open(path_in);

	pos = path_in.find_last_of("/");

	if(pos != string::npos)
		device_short = path_in.substr(pos + 1);
	else
		device_short = path_in;

	_set_shortname(string("usb:") + device_short + ":elv");
	_set_longname(string("ELV I2C interface at ") + path_in);

	_probe();
}

void InterfaceELV::_open(string device_node) throw(exception)
{
    int             result;
    struct termios  tio;

    if((_fd = ::open(device_node.c_str(), O_RDWR | O_NOCTTY | O_EXCL, 0)) < 0)
		throw(minor_exception(string("InterfaceELV::_open: cannot open device ") + device_node));

	ioctl(_fd, TIOCEXCL, 1);

    if(ioctl(_fd, TIOCMGET, &result))
	{
		::close(_fd);
		throw(minor_exception("InterfaceELV::_open: error in TIOCMGET"));
	}

    result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

    if(ioctl(_fd, TIOCMSET, &result))
	{
		::close(_fd);
		throw(minor_exception("InterfaceELV::_open: error in TIOCMSET"));
	}

    result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

    if(ioctl(_fd, TIOCMSET, &result))
	{
		::close(_fd);
		throw(minor_exception("InterfaceELV::_open: error in TIOCMSET"));
	}

    if(tcgetattr(_fd, &tio) == 1)
	{
		::close(_fd);
		throw(minor_exception("InterfaceELV::_open: error in tcgetattr"));
	}

    tio.c_iflag &= ~(BRKINT | INPCK | INLCR | IGNCR | IUCLC |
                    IXON    | IXOFF | IXANY | IMAXBEL | ISTRIP | ICRNL);
    tio.c_iflag |=  (IGNBRK | IGNPAR);

    tio.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONOCR | ONLRET | OFILL | ONLCR);
    tio.c_oflag |=  0;

    tio.c_cflag &=  ~(CSIZE | PARENB | PARODD   | HUPCL | CRTSCTS);
    tio.c_cflag |=  (CREAD | CS8 | CSTOPB | CLOCAL);

    tio.c_lflag &= ~(ISIG   | ICANON    | XCASE | ECHO  | ECHOE | ECHOK |
                    ECHONL | ECHOCTL    | ECHOPRT | ECHOKE | FLUSHO | TOSTOP |
                    PENDIN | IEXTEN     | NOFLSH);
    tio.c_lflag |=  0;

    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);

    if(tcsetattr(_fd, TCSANOW, &tio) == 1)
	{
		::close(_fd);
		throw(minor_exception("InterfaceELV::_open: error in tcsetattr"));
	}

	string rv;

	try
	{
		rv = command("z 4b", 5000, 2);
	}
	catch(minor_exception e)
	{
		dlog("open: minor exception during reset: %s\n", e.message.c_str());
		::close(_fd);
		throw(minor_exception("Interface::ELV: not found"));
	}
	catch(major_exception e)
	{
		dlog("open: major exception during reset: %s\n", e.message.c_str());
		::close(_fd);
		throw(minor_exception("Interface::ELV: not found"));
	}

	if(rv.find("ELV USB-I2C-Interface v1.6") == string::npos)
	{
		dlog("InterfaceELV: id string not recognised\n");
		::close(_fd);
		throw(minor_exception("interface ELV not found"));
	}
}

string InterfaceELV::_command(const string &cmd_in, int timeout, int chunks) throw(exception)
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
		throw(minor_exception("InterfaceELV::_command: clock_gettime error\n"));

	cmd = string(":") + cmd_in + string("\n");

	dlog("> %s", cmd.c_str());

	while(true)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(minor_exception("InterfaceELV::_command: clock_gettime error\n"));

		timeout_left = timespec_diff(start, now);

		if((timeout > 0) && (timeout_left < 0))
			throw(minor_exception("InterfaceELV::_command: fatal timeout (1)"));

		pfd.fd		= _fd;
		pfd.events	= POLLIN | POLLERR;

		pr = poll(&pfd, 1, 0);

		if(pr < 0)
			throw(major_exception("InterfaceELV::_command: poll error (1)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(major_exception("InterfaceELV::_command: poll error (2)"));

		if(pfd.revents & POLLIN)
		{
			len = ::read(_fd, buffer, sizeof(buffer) - 1);
			buffer[len] = 0;
			dlog("clearing backlog, cleared %d bytes: %s\n", len, buffer);
		}
	}

	if(clock_gettime(CLOCK_MONOTONIC, &now))
		throw(minor_exception("InterfaceELV::_command: clock_gettime error\n"));

	timeout_left = timeout > 0 ? timeout - timespec_diff(start, now) : -1;

	pfd.fd		= _fd;
	pfd.events	= POLLOUT | POLLERR;

	pr = poll(&pfd, 1, timeout_left);

	if(pr < 0)
		throw(major_exception("InterfaceELV::_command: poll error (3)"));

	if(pfd.revents & POLLERR)
		throw(major_exception("InterfaceELV::_command: poll error (4)"));

	if(!(pfd.revents & POLLOUT))
		throw(minor_exception("InterfaceELV::_command: fatal timeout (2)"));

	if(::write(_fd, cmd.c_str(), cmd.length()) != (ssize_t)cmd.length())
		throw(major_exception("InterfaceELV::_command: write error"));

	while(chunks > 0)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(minor_exception("InterfaceELV::_command: clock_gettime error\n"));

		timeout_left = timeout > 0 ? timeout - timespec_diff(start, now) : -1;

		if((timeout > 0) && (timeout_left < 0))
			break;

		pfd.fd		= _fd;
		pfd.events	= POLLIN | POLLERR;

		//dlog("read poll, timeout = %d\n", timeout_left);

		pr = poll(&pfd, 1, timeout_left);

		if(pr < 0)
			throw(major_exception("InterfaceELV::_command: poll error (5)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(major_exception("InterfaceELV::_command: poll error (6)"));

		if(pfd.revents & POLLIN)
		{
			len = ::read(_fd, buffer, sizeof(buffer) - 1);
			buffer[len] = 0;
			//dlog("received %d bytes: %s\n", len, buffer);
			rv += buffer;
			chunks--;
		}
	}

	dlog("< %s", rv.c_str());

	return(rv);
}

void InterfaceELV::_probe() throw()
{
	_probe_atmel(0x02);
	_probe_atmel(0x03);
	_probe_tmp275(0x49);
	_probe_digipicco(0x78);
	_probe_tsl2550(0x39);
	_probe_ds1731(0x48);
}

void InterfaceELV::_probe_atmel(int address) throw()
{
	DeviceAtmel		*device;
	string 			error;

	device = 0;
	dlog("probing atmel@0x%02x\n", address);

	try
	{
		device = new DeviceAtmel(&_devices, _generation + 1, _id, _enumerator, path(), address);
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
		dlog("atmel@0x%02x found: %s\n", address, device->shortname().c_str());
		_enumerator++;
		_devices.add(device);
	}
	else
		dlog("atmel@0x%02x not found: %s\n", address, error.c_str());
}

void InterfaceELV::_probe_tmp275(int address) throw()
{
	DeviceTMP275	*device;
	string 			error;

	device = 0;
	dlog("probing tmp275@0x%02x\n", address);

	try
	{
		device = new DeviceTMP275(&_devices, _generation + 1, _id, _enumerator, path(), address);
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
		dlog("tmp275@0x%02x found: %s\n", address, device->shortname().c_str());
		_devices.add(device);
		_enumerator++;
	}
	else
		dlog("%s\n", error.c_str());
}

void InterfaceELV::_probe_digipicco(int address) throw()
{
	DeviceDigipicco	*device;
	string 			error;

	device = 0;
	dlog("probing digipicco@0x%02x\n", address);

	try
	{
		device = new DeviceDigipicco(&_devices, _generation + 1, _id, _enumerator, path(), address);
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
		dlog("digipicco@0x%02x found: %s\n", address, device->shortname().c_str());
		_devices.add(device);
		_enumerator++;
	}
	else
		dlog("%s\n", error.c_str());
}

void InterfaceELV::_probe_tsl2550(int address) throw()
{
	DeviceTSL2550	*device = 0;
	string 			error;

	dlog("probing tsl2550@0x%02x\n", address);

	try
	{
		device = new DeviceTSL2550(&_devices, _generation + 1, _id, _enumerator, path(), address);
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
		dlog("tsl2550@0x%02x found: %s\n", address, device->shortname().c_str());
		_devices.add(device);
		_enumerator++;
	}
	else
		dlog("%s\n", error.c_str());
}

void InterfaceELV::_probe_ds1731(int address) throw()
{
	DeviceDS1731	*device = 0;
	string 			error;

	dlog("probing tsl1731@0x%02x\n", address);

	try
	{
		device = new DeviceDS1731(&_devices, _generation + 1, _id, _enumerator, path(), address);
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
		dlog("ds1731@0x%02x found: %s\n", address, device->shortname().c_str());
		_devices.add(device);
		_enumerator++;
	}
	else
		dlog("%s\n", error.c_str());
}
