#include "id.h"
#include "util.h"
#include "cppstreams.h"

ID::ID() throw()
	: interface(0), device(0), control_type(0), control_index(0)
{
}

ID::ID(const ID & in) throw()
	: interface(in.interface), device(in.device), control_type(in.control_type), control_index(in.control_index)
{
}

ID::ID(uint8_t interface_in, uint8_t device_in, uint8_t control_type_in, uint8_t control_index_in) throw()
	: interface(interface_in), device(device_in), control_type(control_type_in), control_index(control_index_in)
{
}

ID::ID(string id_str) throw()
{
	istringstream	conv;
	uint32_t		id_int;

	conv.str(id_str);
	conv >> hex >> id_int;

	interface		= (id_int & 0xff000000) >> 24;
	device			= (id_int & 0x00ff0000) >> 16;
	control_type	= (id_int & 0x0000ff00) >>  8;
	control_index	= (id_int & 0x000000ff) >>  0;

	//Util::dlog("** %s becomes %02x/%02x/%02x/%02x\n",
			//id_str.c_str(), interface, device, control_type, control_index);
}

ID::operator uint32_t() const throw()
{
	uint32_t rv = 0;

	rv |= interface		<< 24;
	rv |= device		<< 16;
	rv |= control_type	<<  8;
	rv |= control_index	<<  0;

	return(rv);
}

ID::operator string() const throw()
{
	ostringstream conv;
	conv << hex << setw(8) << setfill('0') << this->operator uint32_t();
	return(conv.str());
}

bool ID::operator <(uint32_t b) const throw()
{
	uint32_t a = uint32_t(*this);

	return(a < b);
}

bool ID::operator <(const ID & ib) const throw()
{
	uint32_t a = uint32_t(*this);
	uint32_t b = uint32_t(ib);

	return(a < b);
}

bool ID::operator ==(uint32_t b) const throw()
{
	uint32_t a = uint32_t(*this);

	return(a == b);
}

bool ID::operator ==(const ID & ib) const throw()
{
	uint32_t a = uint32_t(*this);
	uint32_t b = uint32_t(ib);

	return(a == b);
}

bool ID::operator !=(uint32_t b) const throw()
{
	uint32_t a = uint32_t(*this);

	return(a != b);
}

bool ID::operator !=(const ID & ib) const throw()
{
	uint32_t a = uint32_t(*this);
	uint32_t b = uint32_t(ib);

	return(a != b);
}

bool ID::operator >(uint32_t b) const throw()
{
	uint32_t a = uint32_t(*this);

	return(a > b);
}

bool ID::operator >(const ID & ib) const throw()
{
	uint32_t a = uint32_t(*this);
	uint32_t b = uint32_t(ib);

	return(a > b);
}
