#ifndef _interface_h_
#define _interface_h_

#include <unistd.h>

#include <string>
using std::string;

#include <pthread.h>
#include <stdint.h>

#include "devices.h"
#include "id.h"
#include "exception.h"
#include "util.h"

class Interfaces;

class Interface
{
	friend class Interfaces;
	friend class InterfaceUSBraw;
	friend class Device;
	friend class DeviceAtmel;
	friend class DeviceK8055;
	friend class DeviceTMP275;

	public:

		ID	const		id;

						Interface(Interfaces *parent, ID id)	throw(exception);
		virtual			~Interface()							throw();

		Devices *		interface_devices()						throw();

		virtual	string	name_short()							throw()	= 0;
		virtual	string	name_long()								throw()	= 0;
		virtual	string 	interface_id()							throw()	= 0;

	protected:

		pthread_mutex_t		mutex;

		Interfaces* const	root;
		Devices				devices;
		int 				enumerator;

	private:
				void	lock()												throw(exception);
				void	unlock()											throw(exception);

		virtual void	probe_all_devices()									throw(exception) = 0;
		virtual	string	device_interface_desc(void *pdata)					throw() = 0;
		virtual	ssize_t	write_data(void *device_private_data,
						const ByteArray &data, int timeout)					throw(exception) = 0;
		virtual	ssize_t read_data(void *device_private_data,
						ByteArray &data, size_t length, int timeout)		throw(exception) = 0;
		virtual void	release_device(
						void **device_private_data)							throw() = 0;
};
#endif
