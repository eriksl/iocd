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
		friend class DeviceI2C;

				InterfaceELV(Interfaces *root, ID id, string device_node)	throw(exception);
		virtual	~InterfaceELV()												throw();

		static string name_short_static()	throw();
		static string name_long_static()	throw();

		string	name_short()										const	throw();
		string	name_long()											const	throw();
		string	interface_id()										const	throw();

	private:

		struct cmd_t
		{
			string	in;
			string	out;
			int		timeout;
			int		chunks;
		};

		string	device_node;

		void	open(string path)								throw(exception);
		void	interface_command(void *)						throw(exception);
		void	find_devices()									throw();
		template<class DeviceT> void probe_device(int address)	throw();
};

#endif
