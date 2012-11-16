#ifndef _device_k8055_h_
#define _device_k8055_h_

#include "id.h"
#include "exception.h"
#include "device_usbraw.h"

#include <string>
using std::string;

#include <bitset>
using std::bitset;

#include "libusb.h"

class Interfaces;
class Control;

class DeviceK8055 : public DeviceUSBraw
{
	public:

		friend class InterfaceUSBraw;

				DeviceK8055(Interfaces *root, ID,
					libusb_device *dev)					throw(exception);
		virtual	~DeviceK8055()							throw();

		string	name_short()					const	throw();
		string	name_long()						const	throw();

	private:

		typedef enum
		{
			control_input_counter,
			control_input_digital,
			control_input_analog,
			control_output_digital,
			control_output_analog,
		} k8055_control_type;

		typedef enum
		{
			DIGITAL_INP_OFFSET		= 0,
			DIGITAL_OUT_OFFSET		= 1,
			ANALOG_1_OFFSET			= 2,
			ANALOG_2_OFFSET			= 3,
			COUNTER_1_OFFSET		= 4,
			COUNTER_2_OFFSET		= 6,
		} k8055_packet_offset;

		typedef enum
		{
			CMD_RESET				= 0x00,
			CMD_SET_DEBOUNCE_1		= 0x01,
			CMD_SET_DEBOUNCE_2		= 0x01,
			CMD_RESET_COUNTER_1		= 0x03,
			CMD_RESET_COUNTER_2		= 0x04,
			CMD_SET_ANALOG_DIGITAL	= 0x05,
		} k8055_command;

		typedef bitset<8>	bitset8;

		bitset8	digital_inputs;
		uint8_t	analog_inputs[2];
		uint8_t	counters[2];

		bitset8	digital_outputs;
		uint8_t	analog_outputs[2];

		void	update_inputs(void)						throw(exception);
		void	update_outputs(void)					throw(exception);

		double	read(Control *)							throw(exception);
		void	write(Control *, double)				throw(exception);
		double	readwrite(Control *, double)			throw(exception);
		int		readcounter(Control *)					throw(exception);
		int		readresetcounter(Control *)				throw(exception);
		void	find_controls()							throw(exception);

		static string name_short_static()				throw();
		static string name_long_static()				throw();

};
#endif
