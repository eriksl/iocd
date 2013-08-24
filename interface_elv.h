#ifndef _interface_elv_h_
#define _interface_elv_h_

#include <string>
using std::string;

#include "interface.h"

class Interfaces;

class InterfaceELV : public Interface
{
	public:

		friend class Interfaces;

				InterfaceELV(Interfaces *root, ID id, string device_node)	throw(exception);
		virtual	~InterfaceELV()												throw();

		static string name_short_static()									throw();
		static string name_long_static()									throw();

		string	name_short()												throw();
		string	name_long()													throw();
		string	interface_id()												throw();

	private:

		string	device_node;
		int		fd;

		struct if_usbelv_pdata_t
		{
			int	address;
		};

		void	open(string path)											throw(exception);
		void	write_raw(size_t length, const uint8_t *data, int timeout)	throw(exception);
		void	read_raw(size_t *length, uint8_t *data, int timeout)		throw(exception);

		template<class DeviceT> void probe_single_device(int address)		throw();

		virtual	void	probe_all_devices()									throw();
		virtual	string	device_interface_desc(void *device_private_data)	throw();
		virtual	ssize_t	write_data(void *pdata,
						const ByteArray &data, int timeout)					throw(exception);
		virtual	ssize_t read_data(void *device_private_data,
						ByteArray &data, size_t length, int timeout)		throw(exception);
		virtual	void	release_device(
						void **device_private_data)							throw();


};

#endif
