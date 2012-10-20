#ifndef _interface_h_
#define _interface_h_

class Interfaces;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <pthread.h>
#include <stdint.h>

#include "devices.h"
#include "identity.h"

class Interface : public Identity
{
	public:
		typedef vector<uint8_t> byte_array;

				Interface(Interfaces *parent_interfaces, int generation,
						int parent_id, int ordinal, string parent_path, string path)	throw(string);
		virtual	~Interface()															throw();

		Interfaces *	interfaces()											const	throw();
		Devices *		devices()														throw();

		string			command(string cmd, int timeout = 200, int chunks = 1)			throw(string);
		static int		parse_bytes(string str, byte_array & values)					throw();
		static int		timespec_diff(timespec from, timespec to)						throw();

	protected:

		int				_fd;
		int				_enumerator;
		Devices			_devices;
		Interfaces 		*_interfaces;

	private:

		pthread_mutex_t	_mutex;
		bool			_mutex_valid;
		void			_lock()															throw(string);
		void			_unlock()														throw(string);
		virtual	string	_command(const string &cmd, int timeout, int chunks)			throw(string) = 0;
};
#endif
