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
						int min, int max, string unit)	throw(string);
		virtual	~Control()								throw();

		virtual	int		read()					throw(string);
		virtual	void	write(int)				throw(string);
		virtual	int		readwrite(int)			throw(string);
		virtual	int		readcounter()			throw(string);
		virtual	int		readresetcounter()		throw(string);
		virtual int		readpwmmode()			throw(string);
		virtual	void	writepwmmode(int)		throw(string);

		int		min()					const	throw();
		int		max()					const	throw();
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
		int				_min;
		int				_max;
		string			_unit;
		control_props_t	_properties;

	private:
};

#endif
