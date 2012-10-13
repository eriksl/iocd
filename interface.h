#ifndef _interface_h_
#define _interface_h_

class Interfaces;
class Devices;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <pthread.h>
#include <stdint.h>

class Interface
{
	public:
		typedef vector<uint8_t> byte_array;

				Interface(Interfaces * parent_interfaces, const string &id)							throw(string);
		virtual	~Interface()																		throw();

		virtual	string	name()															const	throw() = 0;
		virtual	string	bus()															const	throw() = 0;
		virtual	string	command(const string &cmd, int timeout = 200, int chunks = 1)	const	throw(string) = 0;

		void			lock()																	throw(string);
		void			unlock()																throw(string);
		Interfaces *	interfaces()													const	throw();
		Devices *		devices()														const	throw();

		static int		parse_bytes(string str, byte_array & values)							throw();
		static int		timespec_diff(timespec from, timespec to)								throw();

	protected:

		string			_id;
		int				_fd;
		Interfaces 		*_interfaces;
		Devices			*_devices;

	private:

		pthread_mutex_t	_mutex;
		bool			_mutex_valid;

};
#endif
