#ifndef _interface_elv_h_
#define _interface_elv_h_

#include <string>
using std::string;

#include "interface.h"
#include "if_private_data.h"

class Interfaces;
class InterfaceELV;

class InterfaceELVPrivateData : public InterfacePrivateData
{
	friend	InterfaceELV;

	public:
				InterfaceELVPrivateData()	throw();
		virtual	~InterfaceELVPrivateData()	throw();

	private:

		int		address;
};

class InterfaceELV : public Interface
{
	public:

		friend class Interfaces;

				InterfaceELV(Interfaces *root, ID id, string device_node)	throw(exception);
		virtual	~InterfaceELV()												throw();

		virtual	string	name_short()	const	throw();
		virtual	string	name_long()		const	throw();
		virtual	string	interface_id()	const	throw();

	private:

		string	device_node;
		int		fd;

		void	open(string path)											throw(exception);
		void	write_raw(size_t length, const uint8_t *data, int timeout)	throw(exception);
		void	read_raw(size_t *length, uint8_t *data, int timeout)		throw(exception);

		template<class DeviceT> void probe_single_device(int address)		throw();

		virtual	void	probe_all_devices()									throw();
		virtual	string	device_interface_desc(const InterfacePrivateData &)	throw();
		virtual	ssize_t	write_data(const InterfacePrivateData &,
						const ByteArray &data, int timeout)					throw(exception);
		virtual	ssize_t read_data(const InterfacePrivateData &,
						ByteArray &data, size_t length, int timeout)		throw(exception);
};

#endif
