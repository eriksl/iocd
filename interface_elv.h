#ifndef _interface_elv_h_
#define _interface_elv_h_

#include "interface.h"

class InterfaceELV : public Interface
{
	public:

		InterfaceELV(Interfaces *parent_interfaces, const Identity &id,
				string device_node)				throw(exception);

	protected:

		string	_command(const string &cmd, int timeout, int chunks)	throw(exception);

	private:

		void	_open(string path)				throw(exception);
		void	_probe()						throw();
		void	_probe_atmel(int address)		throw();
		void	_probe_tmp275(int address)		throw();
		void	_probe_digipicco(int address)	throw();
		void	_probe_tsl2550(int address)		throw();
		void	_probe_ds1731(int address)		throw();
};

#endif
