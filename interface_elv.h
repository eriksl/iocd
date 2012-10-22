#ifndef _interface_elv_h_
#define _interface_elv_h_

#include "interface.h"

class InterfaceELV : public Interface
{
	public:

		InterfaceELV(Interfaces *parent_interfaces,
				int generation, int parent_id, int ordinal,
				string parent_path, string path)						throw(exception);

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
