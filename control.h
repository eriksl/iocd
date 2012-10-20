#ifndef _control_h_
#define _control_h_

#include <string>
using std::string;

class Controls;

#include "identity.h"

class Control : public Identity
{
	public:

				Control(Controls *parent_controls,
						int generation, int parent_id, int ordinal, string parent_path,
						double min, double max, string unit, int precision) throw(string);
		virtual	~Control()								throw();

		virtual	double	read()					throw(string);
		virtual	void	write(double)			throw(string);
		virtual	double	readwrite(double)		throw(string);
		virtual	int		readcounter()			throw(string);
		virtual	int		readresetcounter()		throw(string);
		virtual int		readpwmmode()			throw(string);
		virtual	void	writepwmmode(int)		throw(string);
		virtual string	readpwmmode_string()	throw(string);

		string	read_string()					throw(string);
		string	readwrite_string(double)		throw(string);
		string	readcounter_string()			throw(string);
		string	readresetcounter_string()		throw(string);

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

	private:

		static string	_int_to_string(int value)							throw();
		static string	_float_to_string(double value, int precision = 0)	throw();
};

#endif
