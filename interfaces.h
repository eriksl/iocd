#ifndef _interfaces_h_
#define _interfaces_h_

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "exception.h"

class Interface;
class Device;
class Control;

#include <pthread.h>

class Interfaces
{
	public:

		typedef enum
		{
			signal_none,
			signal_user_restart,
			signal_user_quit,
			signal_user_keyint,
			signal_user_keyquit,
			signal_major_error
		} signal_t;

		typedef vector<Interface *>		interfaces_t;
		typedef interfaces_t::iterator	iterator;

		Interfaces()								throw(exception);
		virtual ~Interfaces()						throw();

		iterator	begin()							throw();
		iterator	end()							throw();
		size_t		count()							throw();
		Interface*	find(string id)					throw(exception);
		Device*		find_device(string id)			throw(exception);
		Control*	find_control(string id)			throw(exception);
		Control*	find_control_by_name(string id)	throw(exception);

		signal_t	wait()							throw();
		void		signal(signal_t value)			throw();

	protected:

		int				_enumerator;
		interfaces_t	_interfaces;

	private:

		signal_t		_signal_value;
		pthread_mutex_t	_signal_mutex;
		pthread_cond_t	_signal_condition;

		static	Interfaces	*__instance;
		static	void		__sigint(int);
		static	void		__sigquit(int);

		void	_probe()			throw();
		void	_probe_usb()		throw();
		void	_probe_usb_elv()	throw();
};

#endif
