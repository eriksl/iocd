#ifndef _interface_elv_h_
#define _interface_elv_h_

#include "interface.h"

class InterfaceELV : public Interface
{
	public:

		InterfaceELV(Interfaces *parent_interfaces,
				int generation, int parent_id, int ordinal,
				string parent_path, string path)						throw(string);

	protected:

		string	_command(const string &cmd, int timeout, int chunks)	throw(string);

	private:

		void	_open(string path)			throw(string);
		void	_probe()					throw();
		void	_probe_atmel()				throw();
};

#endif
