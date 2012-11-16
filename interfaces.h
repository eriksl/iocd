#ifndef _interfaces_h_
#define _interfaces_h_

#include <string>
using std::string;

#include <map>
using std::map;

#include "exception.h"
#include "id.h"

class Interface;
class Device;
class Control;

#include <pthread.h>
#include <stdint.h>

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

		typedef map<ID, Interface *>		interfaces_t;
		typedef interfaces_t::iterator		iterator;

					Interfaces()					throw(exception);
		virtual		~Interfaces()					throw();

		iterator	begin()							throw();
		iterator	end()							throw();
		size_t		count()							throw();
		void		clear()							throw();

		Interface*	find_interface(ID)				throw(exception);
		Device*		find_device(ID)					throw(exception);
		Control*	find_control(ID)				throw(exception);

		signal_t	wait()							throw();
		void		signal(signal_t value)			throw();

		static	Interfaces *instance;

	private:

		int					enumerator;
		interfaces_t		interfaces;
		signal_t			signal_value;
		pthread_mutex_t		signal_mutex;
		pthread_cond_t		signal_condition;

									void	probe_interfaces(void)				throw();
		template<class InterfaceT>	void	probe_interface(string device)		throw();
		template<class InterfaceT>	void	probe_interface_0()					throw();
		template<class InterfaceT>	void	probe_interface_1(string device)	throw();

		static	void		sigint(int);
		static	void		sigquit(int);
};

#endif
