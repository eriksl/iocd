#include "device_k8055.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

#include <string.h>

DeviceK8055::DeviceK8055(Interfaces *root_in, ID id_in, libusb_device *dev_in) throw(exception)
	:	DeviceUSBraw(root_in, id_in, dev_in, 0x01, 0x81)
{
	ssize_t	rv;
	uint8_t packet[8];

	memset(packet, 0, sizeof(packet));
	packet[0] = reset;

	if((rv = send_command(sizeof(packet), packet)) != sizeof(packet))
		throw(minor_exception("DD k8055: error resetting device"));

	analog_outputs[0] = analog_outputs[1] = 0;
	digital_outputs = 0;
}

DeviceK8055::~DeviceK8055() throw()
{
}

string DeviceK8055::name_short_static() throw()
{
	return("k8055");
}

string DeviceK8055::name_long_static() throw()
{
	return("Velleman K8055 USB I/O card");
}

string DeviceK8055::name_short() const throw()
{
	return(name_short_static());
}

string DeviceK8055::name_long() const throw()
{
	return(name_long_static());
}

void DeviceK8055::find_controls() throw(exception)
{
	Control	*control;
	int		ix;

	for(ix = 0; ix < 2; ix++)
	{
		control = 0;

		try
		{
			Control::capset cp;
			cp.set(Control::cap_isdigital);
			cp.set(Control::cap_canread);
			cp.set(Control::cap_cancount);

			control = new Control(root, ID(id.interface, id.device, control_input_counter + 1, ix + 1),
					0, 0xffff, "", 0, cp, control_input_counter, ix,
					string("dincnt"), string("Digital input/counter"));
		}
		catch(minor_exception e)
		{
			Util::vlog("DD k8055 find_controls: %s\n", e.message.c_str());
		}

		if(control)
			controls.add(control);
	}

	for(ix = 0; ix < 3; ix++)
	{
		control = 0;

		try
		{
			Control::capset cp;
			cp.set(Control::cap_isdigital);
			cp.set(Control::cap_canread);

			control = new Control(root, ID(id.interface, id.device, control_input_digital + 1, ix + 1),
					0, 1, "", 0, cp, control_input_digital, ix,
					string("din"), string("Digital input"));
		}
		catch(minor_exception e)
		{
			Util::vlog("DD k8055 find_controls: %s\n", e.message.c_str());
		}

		if(control)
			controls.add(control);
	}

	for(ix = 0; ix < 2; ix++)
	{
		control = 0;

		try
		{
			Control::capset cp;
			cp.set(Control::cap_canread);

			control = new Control(root, ID(id.interface, id.device, control_input_analog + 1, ix + 1),
					0, 0xff, "", 0, cp, control_input_analog, ix,
					string("ain"), string("Analog input"));
		}
		catch(minor_exception e)
		{
			Util::vlog("DD k8055 find_controls: %s\n", e.message.c_str());
		}

		if(control)
			controls.add(control);
	}

	for(ix = 0; ix < 8; ix++)
	{
		control = 0;

		try
		{
			Control::capset cp;
			cp.set(Control::cap_isdigital);
			cp.set(Control::cap_canread);
			cp.set(Control::cap_canwrite);

			control = new Control(root, ID(id.interface, id.device, control_output_digital + 1, ix + 1),
					0, 1, "", 0, cp, control_output_digital, ix,
					string("dout"), string("Digital output"));
		}
		catch(minor_exception e)
		{
			Util::vlog("DD k8055 find_controls: %s\n", e.message.c_str());
		}

		if(control)
			controls.add(control);
	}

	for(ix = 0; ix < 2; ix++)
	{
		control = 0;

		try
		{
			Control::capset cp;
			cp.set(Control::cap_canread);
			cp.set(Control::cap_canwrite);
			cp.set(Control::cap_canhardpwm);

			control = new Control(root, ID(id.interface, id.device, control_output_analog + 1, ix + 1),
					0, 0xff, "", 0, cp, control_output_analog, ix,
					string("aout"), string("Analog output"));
		}
		catch(minor_exception e)
		{
			Util::vlog("DD k8055 find_controls: %s\n", e.message.c_str());
		}

		if(control)
			controls.add(control);
	}
}

void DeviceK8055::update_inputs(void) throw(exception)
{
	uint8_t	packet[8];
	ssize_t	rv;

	if((rv = receive_command(sizeof(packet), packet)) != sizeof(packet))
		throw(major_exception("DD k8055: error reading from device"));

	digital_inputs =
	(
		((packet[DIGITAL_INP_OFFSET] >> 4) & 0x03) |	/* Input 1 and 2 */
		((packet[DIGITAL_INP_OFFSET] << 2) & 0x04) |	/* Input 3 */
		((packet[DIGITAL_INP_OFFSET] >> 3) & 0x18)		/* Input 4 and 5 */
	);

	analog_inputs[0]	= packet[ANALOG_1_OFFSET];
	analog_inputs[1]	= packet[ANALOG_2_OFFSET];
	counters[0]			= (packet[COUNTER_1_OFFSET + 1] << 8) | (packet[COUNTER_1_OFFSET + 0]);
	counters[1]			= (packet[COUNTER_2_OFFSET + 1] << 8) | (packet[COUNTER_2_OFFSET + 0]);
}

void DeviceK8055::update_outputs(void) throw(exception)
{
	ssize_t	rv;
	uint8_t	packet[8];

	packet[0] = set_analog_digital;
	packet[1] = digital_outputs.to_ulong();
	packet[2] = analog_outputs[0];
	packet[3] = analog_outputs[1];

	if((rv = send_command(sizeof(packet), packet)) != sizeof(packet))
		throw(major_exception("DD k8055: error writing to device (1)"));

	memset(packet, 0, sizeof(packet));

	if((rv = receive_command(sizeof(packet), packet)) != sizeof(packet))
		throw(major_exception("DD k8055: error writing to device (2)"));

	if(!(packet[1] & 0x01))
		throw(major_exception("DD k8055: error writing to device (3)"));
}

double DeviceK8055::read(Control *control) throw(exception)
{
	int	ordinal = control->index;

	switch(control->type)
	{
		case(control_input_counter):
		{
			if(ordinal < 0 || ordinal > 1)
				throw(minor_exception("DD k8055: counter index out of range"));

			update_inputs();
			return(double(digital_inputs[ordinal]));
		}

		case(control_input_digital):
		{
			if(ordinal < 0 || ordinal > 2)
				throw(minor_exception("DD k8055: digital input index out of range"));

			update_inputs();
			return(double(digital_inputs[ordinal + 2l]));
		}

		case(control_input_analog):
		{
			if(ordinal < 0 || ordinal > 2)
				throw(minor_exception("DD k8055: analog input index out of range"));

			update_inputs();
			return(double(analog_inputs[ordinal]));
		}

		case(control_output_digital):
		{
			if(ordinal < 0 || ordinal > 7)
				throw(minor_exception("DD k8055: digital output index out of range"));

			return(double(digital_outputs[ordinal]));
		}

		case(control_output_analog):
		{
			if(ordinal < 0 || ordinal > 1)
				throw(minor_exception("DD k8055: analog output index out of range"));

			return(double(analog_outputs[ordinal]));
		}

		default:
		{
			throw(minor_exception("control type out of range"));
		}
	}

	return(-1);
}

void DeviceK8055::write(Control *control, double value) throw(exception)
{
	int	ordinal = control->index;

	if(value < control->min || value > control->max)
		throw(minor_exception("DD k8055:: write digital: value out of range"));

	switch(control->type)
	{
		case(control_output_digital):
		{
			if(ordinal < 0 || ordinal > 7)
				throw(minor_exception("DD k8055: write digital output index out of range"));

			digital_outputs[ordinal] = !!value;

			break;
		}

		case(control_output_analog):
		{
			if(ordinal < 0 || ordinal > 1)
				throw(minor_exception("DD k8055: write analog output index out of range"));

			analog_outputs[ordinal] = value;

			break;
		}

		default:
		{
			throw(minor_exception("control type out of range"));
		}
	}

	update_outputs();
}


double DeviceK8055::readwrite(Control *control, double value) throw(exception)
{
	double prev = this->read(control);
	this->write(control, value);
	return(prev);
}

int DeviceK8055::readcounter(Control *control) throw(exception)
{
	int	ordinal = control->index;

	if(ordinal < 0 || ordinal > 1)
		throw(minor_exception("DD k8055: reset counter index out of range"));

	update_inputs();

	return(double(counters[ordinal]));
}

int DeviceK8055::readresetcounter(Control *control) throw(exception)
{
	int		ordinal = control->index;
	ssize_t	rv;
	uint8_t	packet[8];

	if(ordinal < 0 || ordinal > 1)
		throw(minor_exception("DD k8055: reset counter index out of range"));

	uint8_t counter = readcounter(control);

	packet[0] = ordinal == 0 ? reset_counter_1 : reset_counter_2;

	if((rv = send_command(sizeof(packet), packet)) != sizeof(packet))
		throw(major_exception("DD k8055: error writing to device (4)"));

	memset(packet, 0, sizeof(packet));

	if((rv = receive_command(sizeof(packet), packet)) != sizeof(packet))
		throw(major_exception("DD k8055: error writing to device (5)"));

	if(!(packet[1] & 0x01))
		throw(major_exception("DD k8055: error writing to device (6)"));

	return(counter);
}
