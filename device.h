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

				Device(Interfaces *root, ID id, void *pdata)	throw(exception);
		virtual	~Device()										throw();

		virtual void	find_controls()							throw(exception)	= 0;
		virtual string	name_short()							throw()				= 0;
		virtual	string	name_long()								throw()				= 0;

		Controls*		device_controls()						throw();

	protected:

				void 				*pdata;

				Controls 			controls;
				Interfaces* const	root;

				Interface*			parent()								throw(exception);

		ssize_t	write_data(const ByteArray &data, int timeout)				throw(exception);
		ssize_t write_data(int timeout, int a = -1, int b = -1, int c = -1) throw(exception);
		ssize_t read_data(ByteArray &data, size_t length, int timeout)		throw(exception);

	private:

		virtual	double	read(Control *)							throw(exception);
		virtual	void	write(Control *, double value)			throw(exception);
		virtual double	readwrite(Control *, double value)		throw(exception);
		virtual	int		readcounter(Control *)					throw(exception);
		virtual	int		readresetcounter(Control *)				throw(exception);
		virtual	int		readpwmmode(Control *)					throw(exception);
		virtual	void	writepwmmode(Control *, int value)		throw(exception);
		virtual	string	readpwmmode_string(Control *)			throw(exception);
};
#endif
