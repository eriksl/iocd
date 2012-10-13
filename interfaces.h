#include <vector>
using std::vector;

#include "interface.h"

#ifndef _interfaces_h_
#define _interfaces_h_

class Interfaces
{
	public:

		typedef vector<Interface *>				interfaces_t;
		typedef interfaces_t::const_iterator	const_iterator_t;

				Interfaces()	throw();
		virtual ~Interfaces()	throw();

		const_iterator_t	begin()	const	throw();
		const_iterator_t	end()	const	throw();

	protected:

	private:

		typedef interfaces_t::iterator iterator_t;

		interfaces_t _interfaces;

		void _probe_elv() throw();
};

#endif
