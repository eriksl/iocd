#ifndef _device_h_
#define _device_h_

#include "id.h"
#include "controls.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class Interfaces;
class Interface;
class Control;

class Device
{
	public:

		friend	class Control;
		friend	class Interfaces;

		const	ID	id;

				Device(Interfaces *root, ID id)					throw(exception);
		virtual	~Device()										throw();

		virtual void	find_controls()							throw(exception)	= 0;
		virtual string	name_short()					const	throw()				= 0;
		virtual	string	name_long()						const	throw()				= 0;
		virtual	string	device_id()						const	throw()				= 0;

		Controls*		device_controls()						throw();

	protected:

				Controls 			controls;
				Interfaces* const	root;
		virtual void				command(void *cmd)			throw(exception);

	private:

		virtual	double	read(Control *)							throw(exception);
		virtual	void	write(Control *, double value)			throw(exception);
		virtual double	readwrite(Control *, double value)		throw(exception);
		virtual	int		readcounter(Control *)					throw(exception);
		virtual	int		readresetcounter(Control *)				throw(exception);
		virtual	int		readpwmmode(Control *)					throw(exception);
		virtual	void	writepwmmode(Control *, int value)		throw(exception);
		virtual	string	readpwmmode_string(Control *)			throw(exception);

		Interface*		parent()								throw(exception);
};
#endif
