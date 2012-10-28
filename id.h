#ifndef _id_h_
#define _id_h_

#include <string>
using std::string;

#include <stdint.h>

class ID
{
	public:

		uint8_t	interface;
		uint8_t	device;
		uint8_t	control_type;
		uint8_t	control_index;

		ID()														throw();
		ID(const ID &)												throw();
		ID(uint8_t interface, uint8_t device = 0,
			uint8_t control_type = 0, uint8_t control_index = 0)	throw();
		ID(string)													throw();

				operator uint32_t()			const	throw();
				operator string()			const	throw();

		bool	operator <(uint32_t)		const	throw();
		bool	operator <(const ID &)		const	throw();
		bool	operator ==(uint32_t)		const	throw();
		bool	operator ==(const ID &)		const	throw();
		bool	operator !=(uint32_t)		const	throw();
		bool	operator !=(const ID &)		const	throw();
		bool	operator >(uint32_t)		const	throw();
		bool	operator >(const ID &)		const	throw();

};

#endif
