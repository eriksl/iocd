#ifndef _interface_h_
#define _interface_h_

class Interfaces;

#include <string>
using std::string;

#include <pthread.h>
#include <stdint.h>

#include "devices.h"
#include "identity.h"
#include "exception.h"

class Interface : public Identity
{
	public:

				Interface(Interfaces *parent_interfaces, const Identity& id)			throw(exception);
		virtual	~Interface()															throw();

		Interfaces *	interfaces()											const	throw();
		Devices *		devices()														throw();

		string			command(string cmd, int timeout = 200, int chunks = 1)			throw(exception);

	protected:

		int				_fd;
		int				_enumerator;
		Devices			_devices;
		Interfaces 		*_interfaces;

	private:

		pthread_mutex_t	_mutex;
		bool			_mutex_valid;
		void			_lock()															throw(exception);
		void			_unlock()														throw(exception);
		virtual	string	_command(const string &cmd, int timeout, int chunks)			throw(exception) = 0;
};
#endif
