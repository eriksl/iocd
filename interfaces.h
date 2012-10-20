#ifndef _interfaces_h_
#define _interfaces_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

class Interface;
class Device;
class Control;

class Interfaces
{
	public:

		typedef vector<Interface *>		interfaces_t;
		typedef interfaces_t::iterator	iterator;

		Interfaces()								throw(string);
		virtual ~Interfaces()						throw();

		iterator	begin()							throw();
		iterator	end()							throw();
		Interface*	find(string id)					throw(string);
		Device*		find_device(string id)			throw(string);
		Control*	find_control(string id)			throw(string);
		Control*	find_control_by_name(string id)	throw(string);

	protected:

		int				_enumerator;
		interfaces_t	_interfaces;

	private:

		void	_probe()			throw();
		void	_probe_usb()		throw();
		void	_probe_usb_elv()	throw();
};

#endif
