#ifndef _device_h_
#define _device_h_

#include "identity.h"
#include "controls.h"
#include "exception.h"
#include "util.h"

#include <string>
using std::string;

class Devices;

class Device : public Identity
{
	public:

				Device(Devices *parent, const Identity& id)		throw(exception);
		virtual	~Device()										throw();

		Devices *					devices()										throw();
		Controls *					controls()										throw();
		virtual Util::byte_array	command(string cmd, int timeout, int chunks)	throw(exception);

	protected:

		Devices		*_devices;
		Controls	_controls;
		int			_enumerator;

	private:
};
#endif
