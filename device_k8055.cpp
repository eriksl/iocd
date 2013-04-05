#include "device_k8055.h"
#include "control.h"
#include "cppstreams.h"
#include "util.h"

#include <string.h>
#include <stddef.h>

DeviceK8055::DeviceK8055(Interfaces *root_in, ID id_in, libusb_device *dev_in) throw(exception)
	:	DeviceUSBraw(root_in, id_in, dev_in, 0x01, 0x81)
{
	ssize_t	rv;
	input_packet_t	input_packet;
	output_packet_t	output_packet;

	memset(&output_packet, 0, sizeof(output_packet));

	output_packet.command = reset;

	if((rv = send_command(sizeof(output_packet), (uint8_t *)&output_packet)) != sizeof(output_packet))
		throw(minor_exception("error resetting device (1)"));

	if((rv = receive_command(sizeof(input_packet), (uint8_t *)&input_packet)) != sizeof(input_packet))
		throw(minor_exception("error resetting device (2)"));

	if(!(input_packet.status & 0x01))
		throw(minor_exception("error resetting device (3)"));

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
	input_packet_t	input_packet;
	ssize_t	rv;

	if((rv = receive_command(sizeof(input_packet), (uint8_t *)&input_packet)) != sizeof(input_packet))
		throw(major_exception("DD k8055: error reading from device (1)"));

	if(!(input_packet.status & 0x01))
		throw(major_exception("DD k8055: error reading from device (2)"));

	if((rv = receive_command(sizeof(input_packet), (uint8_t *)&input_packet)) != sizeof(input_packet))
		throw(major_exception("DD k8055: error reading from device (3)"));

	if(!(input_packet.status & 0x01))
		throw(major_exception("DD k8055: error reading from device (4)"));

	digital_inputs =
	(
		((input_packet.digital_input >> 4) & 0x03) |	/* input 1 and 2 */
		((input_packet.digital_input << 2) & 0x04) |	/* input 3 */
		((input_packet.digital_input >> 3) & 0x18)		/* input 4 and 5 */
	);

	analog_inputs[0]	= input_packet.analog_input_0;
	analog_inputs[1]	= input_packet.analog_input_1;
	counters[0]			= input_packet.counter_0;
	counters[1]			= input_packet.counter_1;
}

void DeviceK8055::update_outputs(void) throw(exception)
{
	ssize_t	rv;
	input_packet_t	input_packet;
	output_packet_t	output_packet;

	memset(&output_packet, 0, sizeof(output_packet));

	output_packet.command			= set_analog_digital;
	output_packet.digital_output	= digital_outputs.to_ulong();
	output_packet.analog_output_0	= analog_outputs[0];
	output_packet.analog_output_1	= analog_outputs[1];

	if((rv = send_command(sizeof(output_packet), (uint8_t *)&output_packet)) != sizeof(output_packet))
		throw(major_exception("DD k8055: error writing to device (1)"));

	if((rv = receive_command(sizeof(input_packet), (uint8_t *)&input_packet)) != sizeof(input_packet))
		throw(major_exception("DD k8055: error writing to device (2)"));

	if(!(input_packet.status & 0x01))
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

	return(counters[ordinal]);
}

int DeviceK8055::readresetcounter(Control *control) throw(exception)
{
	int				ordinal = control->index;
	ssize_t			rv;
	int				counter;
	input_packet_t	input_packet;
	output_packet_t	output_packet;

	if(ordinal < 0 || ordinal > 1)
		throw(minor_exception("DD k8055: reset counter index out of range"));

	counter = readcounter(control);

	memset(&output_packet, 0, sizeof(output_packet));

	output_packet.command = ordinal ? reset_counter_1 : reset_counter_0;

	if((rv = send_command(sizeof(output_packet), (uint8_t *)&output_packet)) != sizeof(output_packet))
		throw(major_exception("DD k8055: error writing to device (4)"));

	if((rv = receive_command(sizeof(input_packet), (uint8_t *)&input_packet)) != sizeof(input_packet))
		throw(major_exception("DD k8055: error writing to device (5)"));

	if(!(input_packet.status & 0x01))
		throw(major_exception("DD k8055: error writing to device (6)"));

	return(counter);
}
