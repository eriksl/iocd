#ifndef _interface_elv_h_
#define _interface_elv_h_

#include "interface.h"

class InterfaceELV : public Interface
{
	public:

		InterfaceELV(Interfaces *parent_interfaces, const Identity &id,
				string device_node)				throw(exception);

	protected:

		string	_command(const string &cmd, int timeout, int chunks) throw(exception);

	private:

									void	_open(string path)	throw(exception);
		template<class ControlT>	void	_probe(int address)	throw();
};

#endif
