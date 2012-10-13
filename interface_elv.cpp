#include "devices.h"
#include "interface_elv.h"
#include "device_atmel.h"
#include "syslog.h"

#include "cppstreams.h"

#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>

InterfaceELV::InterfaceELV(Interfaces * interfaces_in, const string &id) throw(string) : Interface(interfaces_in, id)
{
	_open();
	_probe_bus();
}

InterfaceELV::~InterfaceELV() throw()
{
}

string InterfaceELV::name() const throw()
{
	return("ELV I2C interface");
}

string InterfaceELV::bus() const throw()
{
	return(string("usb:") + _id);
}

void InterfaceELV::_open() throw(string)
{
    int             result;
    struct termios  tio;

    if((_fd = ::open(_id.c_str(), O_RDWR | O_NOCTTY | O_EXCL, 0)) < 0)
		throw(string("InterfaceELV::_open: cannot open device ") + _id);

	ioctl(_fd, TIOCEXCL, 1);

    if(ioctl(_fd, TIOCMGET, &result))
	{
		::close(_fd);
		throw(string("InterfaceELV::_open: error in TIOCMGET"));
	}

    result &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

    if(ioctl(_fd, TIOCMSET, &result))
	{
		::close(_fd);
		throw(string("InterfaceELV::_open: error in TIOCMSET"));
	}

    result |= (TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR);

    if(ioctl(_fd, TIOCMSET, &result))
	{
		::close(_fd);
		throw(string("InterfaceELV::_open: error in TIOCMSET"));
	}

    if(tcgetattr(_fd, &tio) == 1)
	{
		::close(_fd);
		throw(string("InterfaceELV::_open: error in tcgetattr"));
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
		throw(string("InterfaceELV::_open: error in tcsetattr"));
	}

	dlog("*** reset\n");

	string rv;

	try
	{
		rv = command("z 4b", 5000, 2);
	}
	catch(string s)
	{
		vlog("open: exception during reset: %s\n", s.c_str());
		::close(_fd);
		throw(string("Interface::ELV: not found"));
	}

	dlog("result: \"%s\"\n", rv.c_str());

	if(rv.find("ELV USB-I2C-Interface v1.6") == string::npos)
	{
		vlog("InterfaceELV: id string not recognised\n");
		::close(_fd);
		throw(string("interface ELV not found"));
	}
}

string InterfaceELV::command(const string &cmd_in, int timeout, int chunks) const throw(string)
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
		throw(string("InterfaceELV::command: clock_gettime error\n"));

	cmd = string(":") + cmd_in + string("\n");

	dlog("> %s", cmd.c_str());

	while(true)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(string("InterfaceELV::command: clock_gettime error\n"));

		timeout_left = timespec_diff(start, now);

		if((timeout > 0) && (timeout_left < 0))
			throw(string("InterfaceELV::command: fatal timeout (1)"));

		pfd.fd		= _fd;
		pfd.events	= POLLIN | POLLERR;

		pr = poll(&pfd, 1, 0);

		if(pr < 0)
			throw(string("InterfaceELV::command: poll error (1)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(string("InterfaceELV::command: poll error (2)"));

		if(pfd.revents & POLLIN)
		{
			len = ::read(_fd, buffer, sizeof(buffer) - 1);
			buffer[len] = 0;
			dlog("clearing backlog, cleared %d bytes: %s\n", len, buffer);
		}
	}

	if(clock_gettime(CLOCK_MONOTONIC, &now))
		throw(string("InterfaceELV::command: clock_gettime error\n"));

	timeout_left = timeout > 0 ? timeout - timespec_diff(start, now) : -1;

	pfd.fd		= _fd;
	pfd.events	= POLLOUT | POLLERR;

	pr = poll(&pfd, 1, timeout_left);

	if(pr < 0)
		throw(string("InterfaceELV::command: poll error (3)"));

	if(pfd.revents & POLLERR)
		throw(string("InterfaceELV::command: poll error (4)"));

	if(!(pfd.revents & POLLOUT))
		throw(string("InterfaceELV::command: fatal timeout (2)"));

	//dlog("write: \"%s\"\n", cmd.c_str());

	if(::write(_fd, cmd.c_str(), cmd.length()) != (ssize_t)cmd.length())
		throw(string("InterfaceELV::command: write error"));

	while(chunks > 0)
	{
		if(clock_gettime(CLOCK_MONOTONIC, &now))
			throw(string("InterfaceELV::command: clock_gettime error\n"));

		timeout_left = timeout > 0 ? timeout - timespec_diff(start, now) : -1;

		if((timeout > 0) && (timeout_left < 0))
			break;

		pfd.fd		= _fd;
		pfd.events	= POLLIN | POLLERR;

		//dlog("read poll, timeout = %d\n", timeout_left);

		pr = poll(&pfd, 1, timeout_left);

		if(pr < 0)
			throw(string("InterfaceELV::command: poll error (5)"));

		if(pr == 0)
			break;

		if(pfd.revents & POLLERR)
			throw(string("InterfaceELV::command: poll error (6)"));

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

void InterfaceELV::_probe_bus() throw()
{
	_probe_atmel();
}

void InterfaceELV::_probe_atmel() throw()
{
	int				address;
	DeviceAtmel		*device;
	string 			error;

	for(address = 0x02; address < 0x04; address++)
	{
		device = 0;
		dlog("probing atmel@0x%02x\n", address);

		try
		{
			device = new DeviceAtmel(this, _devices, address);
		}
		catch(string e)
		{
			error = e;
		}
		catch(...)
		{
		}

		if(device)
		{
			vlog("atmel@0x%02x found: %s\n", address, device->name().c_str());
			_devices->add(device);
		}
		else
		{
			vlog("atmel@0x%02x not found: %s\n", address, error.c_str());
		}
	}
}
