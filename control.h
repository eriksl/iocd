#ifndef _control_h_
#define _control_h_

#include <string>
using std::string;

class Interface;
class Devices;
class Device;
class Controls;

#include "identity.h"
#include "exception.h"
#include "util.h"

class Control : public Identity
{
	public:

				Control(Controls *parent_controls,
						int generation, int parent_id, int ordinal, string parent_path,
						double min, double max, string unit, int precision) throw(exception);
		virtual	~Control()								throw();

		virtual	double	read()					throw(exception);
		virtual	void	write(double)			throw(exception);
		virtual	double	readwrite(double)		throw(exception);
		virtual	int		readcounter()			throw(exception);
		virtual	int		readresetcounter()		throw(exception);
		virtual int		readpwmmode()			throw(exception);
		virtual	void	writepwmmode(int)		throw(exception);
		virtual string	readpwmmode_string()	throw(exception);

		string	read_string()					throw(exception);
		string	readwrite_string(double)		throw(exception);
		string	readcounter_string()			throw(exception);
		string	readresetcounter_string()		throw(exception);

		double	min()					const	throw();
		string	min_string()			const	throw();
		double	max()					const	throw();
		string	max_string()			const	throw();
		int		precision()				const	throw();
		string	precision_string()		const	throw();
		string	unit()					const	throw();
		string	properties()			const	throw();

		bool	canread()				const	throw();
		bool	canwrite()				const	throw();
		bool	cancount()				const	throw();
		bool	canpwm()				const	throw();
		bool	isdigital()				const	throw();

		Controls*	controls()					throw();
		Device*		device()					throw();
		Devices*	devices()					throw();
		Interface*	interface()					throw();

	protected:

		typedef enum
		{
			cp_canread		= 1 << 0,
			cp_canwrite		= 1 << 1,
			cp_cancount		= 1 << 2,
			cp_canpwm		= 1 << 3,
			cp_isdigital	= 1 << 4
		} control_props_t;

		Controls		*_controls;
		double			_min;
		double			_max;
		string			_unit;
		control_props_t	_properties;
		int				_precision;

		Util::byte_array _command(string cmd, int timeout = 200, int chunks = 1) throw(exception); 

	private:

		static string	_int_to_string(int value)							throw();
		static string	_float_to_string(double value, int precision = 0)	throw();
};

#endif
