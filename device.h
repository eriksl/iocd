#ifndef _device_h_
#define _device_h_

#include "interface.h"
#include "identity.h"
#include "controls.h"

#include <string>
using std::string;

class Devices;

class Device : public Identity
{
	public:

				Device(Devices *parent,
						int generation, int parent_id, int ordinal,
						string parent_path)						throw(string);
		virtual	~Device()										throw();
		virtual Interface::byte_array command(string cmd,
				int timeout = 200, int chunks = 1)		const	throw(string) = 0;

		Devices *	devices()									throw();
		Controls *	controls()									throw();

	protected:

		Devices		*_devices;
		Controls	_controls;
		int			_enumerator;

	private:
};
#endif
