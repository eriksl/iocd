#ifndef _control_h_
#define _control_h_

#include <string>
using std::string;

#include <bitset>
using std::bitset;

class Interfaces;
class Device;

#include "id.h"
#include "exception.h"
#include "util.h"

class Control
{
	public:

		enum
		{
			cap_isdigital,
			cap_canread,
			cap_canwrite,
			cap_cancount,
			cap_cansoftpwm,
			cap_canhardpwm,
			cap_canpwmmode,
		};

		typedef bitset<8> capset;

		friend class DeviceAtmel;
		friend class DeviceDigipicco;

				Control(Interfaces *parent_controls, ID,
						double min, double max, string unit, int precision,
						const capset &caps, int type, int index,
						string name, string description)		throw(exception);
		virtual	~Control()										throw();

		const	ID			id;
		const	double		min;
		const	double		max;
		const	string		unit;
		const	int			precision;
		const	int			type;
		const	capset		caps;
		const	int			index;
		const	string		name_short;
		const	string		name_long;
		const	string		control_id;

		string	min_string()			const	throw();
		string	max_string()			const	throw();
		string	precision_string()		const	throw();
		string	capabilities()			const	throw();
		string	type_string()			const	throw();
		string	index_string()			const	throw();

		bool	isdigital()				const	throw();
		bool	canread()				const	throw();
		bool	canwrite()				const	throw();
		bool	cancount()				const	throw();
		bool	canpwm()				const	throw();
		bool	cansoftpwm()			const	throw();
		bool	canhardpwm()			const	throw();

		double	read()							throw(exception);
		void	write(double)					throw(exception);
		double	readwrite(double)				throw(exception);
		int		readcounter()					throw(exception);
		int		readresetcounter()				throw(exception);
		int		readpwmmode()					throw(exception);
		void	writepwmmode(int)				throw(exception);

		string	read_string()					throw(exception);
		string	readwrite_string(double)		throw(exception);
		string	readcounter_string()			throw(exception);
		string	readresetcounter_string()		throw(exception);
		string	readpwmmode_string()			throw(exception);

	private:

		Interfaces* const	root;

		Device*				parent()												throw(exception);
		Util::byte_array	command(string cmd, int timeout = 200, int chunks = 1)	throw(exception); 
};
#endif
