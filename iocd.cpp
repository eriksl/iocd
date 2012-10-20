#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "iocd.h"

#include <string>
using std::string;

#include "interfaces.h"
#include "interface.h"
#include "devices.h"
#include "device.h"
#include "controls.h"
#include "control.h"
#include "syslog.h"
#include "http_server.h"

static bool quit = false;

static void sigint(int)
{
	dlog("SIGINT\n");
	signal(SIGINT, SIG_DFL);
	quit = true;
}

static void log_exception(const string & error)
{
	string errormessage = string("error caught, message = ") + error;

	if(errno != 0)
	{
		errormessage += string(", system error = ");
		errormessage += strerror(errno);
	}

	vlog("%s\n", errormessage.c_str());
}

int main(int argc, char ** argv)
{
	int			opt;
	bool		foreground	= false;
	bool		oneshot		= false;

	try
	{
		while((opt = getopt(argc, argv, "dfo")) != -1)
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

				default:
				{
					errno = 0;
					throw(string("\nusage: iocd [-d] [-f] [-o]\n-f = foreground, -d = debug, -o = oneshot mode"));
				}
			}
		}

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

		signal(SIGINT, sigint);

		if(!foreground)
		{
			isdaemon = true;
			daemon(0, 0);
		}
		else
			isdaemon = false;

		setresuid(65534, 65534, 65534);

		HttpServer *httpserver = new HttpServer(&interfaces, 28000);

		dlog("before pause\n");
		pause();
		dlog("after pause\n");

		delete httpserver;
	}
	catch(const string & error)
	{
		log_exception(error);
		exit(-1);
	}
	catch(const char * error)
	{
		log_exception(string(error));
		exit(-1);
	}
	catch(...)
	{
		log_exception(string("caught unknown error"));
		exit(-1);
	}

	if(quit)
		dlog("interrupt\n");

	return(0);
}
