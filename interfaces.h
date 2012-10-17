#ifndef _interfaces_h_
#define _interfaces_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

class Interface;

class Interfaces
{
	public:

		typedef vector<Interface *>		interfaces_t;
		typedef interfaces_t::iterator	iterator;

		Interfaces()					throw(string);
		virtual ~Interfaces()			throw();

		iterator	begin()				throw();
		iterator	end()				throw();

	protected:

		int				_enumerator;
		interfaces_t	_interfaces;

	private:

		void	_probe()			throw();
		void	_probe_usb()		throw();
		void	_probe_usb_elv()	throw();
};

#endif
