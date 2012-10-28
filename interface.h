#ifndef _interface_h_
#define _interface_h_

#include <string>
using std::string;

#include <pthread.h>
#include <stdint.h>

#include "devices.h"
#include "id.h"
#include "exception.h"

class Interfaces;

class Interface
{
	public:

		friend class Interfaces;
		friend class Device;

		ID	const		id;

						Interface(Interfaces *parent, ID id)	throw(exception);
		virtual			~Interface()							throw();

		virtual	string	name_short()					const	throw()				= 0;
		virtual	string	name_long()						const	throw()				= 0;
		virtual	string 	interface_id()					const	throw()				= 0;

		Devices *		interface_devices()						throw();

	protected:

		Interfaces* const	root;
		Devices				devices;
		int 				fd;
		int 				enumerator;

		string			command(string cmd, int timeout = 200, int chunks = 1)		throw(exception);

	private:

				pthread_mutex_t	mutex;

		virtual void	find_devices()													throw(exception) = 0;
		virtual	string	interface_command(const string &cmd, int timeout, int chunks)	throw(exception) = 0;
				void	lock()															throw(exception);
				void	unlock()														throw(exception);
};
#endif
