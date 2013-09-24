#ifndef _device_k8055_h_
#define _device_k8055_h_

#include "device.h"
#include "id.h"
#include "exception.h"

#include <string>
using std::string;

#include <bitset>
using std::bitset;

class Interfaces;
class Control;

class DeviceK8055 : public Device
{
	public:

				DeviceK8055(Interfaces *root, ID, const InterfacePrivateData *)	throw(exception);
		virtual	~DeviceK8055()													throw();

		string	name_short()	throw();
		string	name_long()		throw();

	private:

		friend class InterfaceUSBraw;
		friend class InterfaceELV;

		enum
		{
			control_input_counter,
			control_input_digital,
			control_input_analog,
			control_output_digital,
			control_output_analog,
		};

		enum
		{
			reset				= 0x00,
			set_debounce_0		= 0x01,
			set_debounce_1		= 0x02,
			reset_counter_0		= 0x03,
			reset_counter_1		= 0x04,
			set_analog_digital	= 0x05,
		};

		struct input_packet_t
		{
			uint8_t		digital_input;
			uint8_t		status;
			uint8_t		analog_input_0;
			uint8_t		analog_input_1;
			uint16_t	counter_0;
			uint16_t	counter_1;
		};

		struct output_packet_t
		{
			uint8_t		command;
			uint8_t		digital_output;
			uint8_t		analog_output_0;
			uint8_t		analog_output_1;
			uint8_t		reset_counter_0;
			uint8_t		reset_counter_1;
			uint8_t		debounce_counter_0;
		};

		typedef bitset<8> bitset8;

		bitset8		digital_inputs;
		int			analog_inputs[2];
		int			counters[2];

		bitset8	digital_outputs;
		uint8_t	analog_outputs[2];

		void	update_inputs(void)							throw(exception);
		void	update_outputs(void)						throw(exception);

		double	read(Control *)								throw(exception);
		void	write(Control *, double)					throw(exception);
		double	readwrite(Control *, double)				throw(exception);
		int		readcounter(Control *)						throw(exception);
		int		readresetcounter(Control *)					throw(exception);

		bool	probe()										throw();
		void	find_controls()								throw(exception);

		void	send_command(const output_packet_t *)		throw(exception);
		void	receive_command(input_packet_t *)			throw(exception);

		static string name_short_static()					throw();
		static string name_long_static()					throw();

};
#endif
