#include "interface.h"

#ifndef _interface_elv_h_
#define _interface_elv_h_

class InterfaceELV : public Interface
{
	public:

		InterfaceELV(Interfaces * parent_interfaces, const string &id)					throw(string);
		virtual ~InterfaceELV()															throw();

		string	name()															const	throw();
		string	bus()															const	throw();
		string	command(const string &cmd, int timeout = 200, int chunks = 1)	const	throw(string);

	protected:

	private:

		void	_open()			throw(string);
		void	_probe_bus()	throw();
		void	_probe_atmel()	throw();
};

#endif
