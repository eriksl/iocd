#include "http_server.h"
#include "interfaces.h"
#include "interface.h"
#include "devices.h"
#include "device.h"
#include "controls.h"
#include "control.h"
#include "exception.h"
#include "util.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <string>
using std::string;

static void log_exception(string type, string error)
{
	string errormessage = string("error caught, type = ") +  type + ", message = " + error;

	if(errno != 0)
	{
		errormessage += string(", system error = ");
		errormessage += strerror(errno);
	}

	Util::vlog("%s\n", errormessage.c_str());
}

int main(int argc, char ** argv)
{
	int						opt;
	bool					foreground	= false;
	bool					oneshot		= false;
	size_t					interfaces_required = 0;
	Interfaces::signal_t	waitval;

	while((opt = getopt(argc, argv, "dfor:")) != -1)
	{
		switch(opt)
		{
			case('d'):
			{
				Util::debug = true;
				foreground = true;
				break;
			}

			case('f'):
			{
				foreground = true;
				break;
			}

			case('o'):
			{
				oneshot = true;
				break;
			}

			case('r'):
			{
				interfaces_required = strtoul(optarg, 0, 10);
				break;
			}

			default:
			{
				errno = 0;
				fprintf(stderr, "usage: iocd [-d] [-f] [-o] [-r interfaces]\n"
								"-f = foreground, -d = debug, -o = oneshot mode,\n"
								"-r # = number of interfaces required to be present\n");
				exit(1);
			}
		}
	}

	if(!foreground)
	{
		Util::isdaemon = true;
		daemon(0, 0);
		setresuid(65534, 65534, 65534);
	}
	else
		Util::isdaemon = false;

	for(;;)
	{
		try
		{
			Interfaces				interfaces;
			Interfaces::iterator	interface;
			Devices::iterator		device;
			Controls::iterator		control;

			for(interface = interfaces.begin(); interface != interfaces.end(); interface++)
			{
				if(interface->second->interface_devices()->begin() == interface->second->interface_devices()->end())
				{

					Util::dlog("DD delete interface: [l:%s] [s:%s] [i:%s] {%s}\n",
						interface->second->name_long().c_str(),
						interface->second->name_short().c_str(),
						interface->second->interface_id().c_str(),
						string(interface->second->id).c_str());

					interfaces.remove(interface->second->id);

					if(interfaces.count() > 0)
						interface = interfaces.begin();
					else
						break;
				}

			}

			for(interface = interfaces.begin(); interface != interfaces.end(); interface++)
			{
				Util::vlog("interface: [l:%s] [s:%s] [i:%s] {%s}\n",
						interface->second->name_long().c_str(),
						interface->second->name_short().c_str(),
						interface->second->interface_id().c_str(),
						string(interface->second->id).c_str());

				for(device = interface->second->interface_devices()->begin(); device != interface->second->interface_devices()->end(); device++)
				{
					Util::vlog("    device: [l:%s] [s:%s] {%s}\n",
							device->second->name_long().c_str(),
							device->second->name_short().c_str(),
							string(device->second->id).c_str());

					for(control = device->second->device_controls()->begin(); control != device->second->device_controls()->end(); control++)
					{
						Util::vlog("        control: [l:%s] [s:%s] [i:%s] [o:%s] [%s] (min:%s-max:%s unit:%s) (props:%s) (value = %s) (counter = %s)\n",
								control->second->name_long.c_str(),
								control->second->name_short.c_str(),
								control->second->control_id.c_str(),
								control->second->index_string().c_str(),
								string(control->second->id).c_str(),
								control->second->min_string().c_str(), control->second->max_string().c_str(), control->second->unit.c_str(),
								control->second->capabilities().c_str(),
								control->second->canread()  ? control->second->read_string().c_str() : "",
								control->second->cancount() ? control->second->readresetcounter_string().c_str() : "");
					}
				}
			}

			if(oneshot)
				throw(oneshot_exception());

			if(interfaces.count() < interfaces_required)
			{
				Util::vlog("insufficient number of interfaces detected\n");
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				sleep(1);
				continue;
			}

			{
				HttpServer httpserver(&interfaces, 28000);

				Util::dlog("before wait\n");
				waitval = interfaces.wait();
				Util::dlog("wait returned %d\n", waitval);
			}

			if(waitval == Interfaces::signal_user_quit)
				throw(fatal_exception("quit by user request"));

			if(waitval == Interfaces::signal_user_restart)
				throw(major_exception("restart by user request"));

			if(waitval == Interfaces::signal_major_error)
				throw(major_exception("restart by major error"));

			if(waitval == Interfaces::signal_user_keyint)
				throw(fatal_exception("quit by user interrupt"));

			if(waitval == Interfaces::signal_user_keyquit)
				throw(major_exception("reload by user interrupt"));
		}
		catch(minor_exception e)
		{
			log_exception("minor", e.message);
		}
		catch(major_exception e)
		{
			log_exception("major", e.message);
		}
		catch(fatal_exception e)
		{
			log_exception("fatal", e.message);
			break;
		}
		catch(oneshot_exception e)
		{
			Util::vlog("done\n");
			break;
		}
		catch(exception e)
		{
			log_exception("generic", e.what());
		}
		catch(string e)
		{
			log_exception("string", e);
		}
		catch(const char * e)
		{
			log_exception("charp", e);
		}
		catch(...)
		{
			log_exception("unknown", "");
			break;
		}
	}

	Util::dlog("exit\n");

	return(0);
}
