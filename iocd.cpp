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
#include "devices.h"
#include "controls.h"
#include "syslog.h"

static bool quit = false;

static void sigint(int)
{
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
	bool		oneshot		= true;

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

		Interfaces interfaces;

		Interfaces::const_iterator_t interface;

		for(interface = interfaces.begin(); interface != interfaces.end(); interface++)
		{
			vlog("interface: %s (%s)\n", (**interface).name().c_str(), (**interface).bus().c_str());

			Devices::const_iterator_t device;

			for(device = (**interface).devices()->begin(); device != (**interface).devices()->end(); device++)
			{
				vlog("    device: %s (%s)\n", (**device).name().c_str(), (**device).bus().c_str());

				Controls::const_iterator_t control;

				for(control = (**device).controls()->begin(); control != (**device).controls()->end(); control++)
					vlog("        control: %s (%s) (%d-%d%s) (%s) (value = %d) (counter = %d)\n",
							(**control).name().c_str(),
							(**control).id().c_str(),
							(**control).min(), (**control).max(), (**control).unit().c_str(),
							(**control).properties().c_str(),
							(**control).canread()  ? (**control).read() : -1,
							(**control).cancount() ? (**control).readresetcounter() : -1);
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

		//HttpServer * http_server = new HttpServer(device, 28000, true);
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
		vlog("interrupt\n");

	return(0);
}
