#include "http_server.h"
#include "interfaces.h"
#include "interface.h"
#include "devices.h"
#include "device.h"
#include "controls.h"
#include "control.h"
#include "syslog.h"
#include "exception.h"
#include "iocd.h"

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

	vlog("%s\n", errormessage.c_str());
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
				debug = true;
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
		isdaemon = true;
		daemon(0, 0);
	}
	else
		isdaemon = false;

	setresuid(65534, 65534, 65534);

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
				vlog("interface: [l:%s] [s:%s] [p:%s] {o:%s/p:%s/i:%s}\n",
						(**interface).longname().c_str(),
						(**interface).shortname().c_str(),
						(**interface).path().c_str(),
						(**interface).ordinal().c_str(),
						(**interface).parent_id().c_str(),
						(**interface).id().c_str());

				for(device = (**interface).devices()->begin(); device != (**interface).devices()->end(); device++)
				{
					vlog("    device: [l:%s] [s:%s] [p:%s] {o:%s/p:%s/i:%s}\n",
							(**device).longname().c_str(),
							(**device).shortname().c_str(),
							(**device).path().c_str(),
							(**device).ordinal().c_str(),
							(**device).parent_id().c_str(),
							(**device).id().c_str());

					for(control = (**device).controls()->begin(); control != (**device).controls()->end(); control++)
					{
						vlog("            control: [l:%s] [s:%s] [p:%s] {o:%s/p:%s/i:%s} (min:%s-max:%s unit:%s) (props:%s) (value = %s) (counter = %s)\n",
								(**control).longname().c_str(),
								(**control).shortname().c_str(),
								(**control).path().c_str(),
								(**control).ordinal().c_str(),
								(**control).parent_id().c_str(),
								(**control).id().c_str(),
								(**control).min_string().c_str(), (**control).max_string().c_str(), (**control).unit().c_str(),
								(**control).properties().c_str(),
								(**control).canread()  ? (**control).read_string().c_str() : "",
								(**control).cancount() ? (**control).readresetcounter_string().c_str() : "");
					}
				}
			}

			if(oneshot)
				exit(0);

			if(interfaces.count() < interfaces_required)
			{
				vlog("insufficient number of interfaces detected\n");
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				sleep(1);
				continue;
			}

			HttpServer httpserver(&interfaces, 28000);

			dlog("before wait\n");
			waitval = interfaces.wait();
			dlog("wait returned %d\n", waitval);

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

	dlog("exit\n");

	return(0);
}
