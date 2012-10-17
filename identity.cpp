#include "identity.h"
#include "cppstreams.h"

Identity::Identity(int generation_in, int parent_id_in, int ordinal_in, string parent_path_in) throw()
	:	_generation(generation_in), _ordinal(ordinal_in), _parent_id(parent_id_in), _parent_path(parent_path_in)
{
	_id	 = (_ordinal & 0xff) << (8 * (3 - _generation));
	_id |= _parent_id;
}

string Identity::generation() const throw()
{
	stringstream conv;
	conv << _generation;
	return(conv.str());
}

string Identity::parent_id() const throw()
{
	stringstream conv;
	conv << hex << setfill('0') << setw(8) << _parent_id;
	return(conv.str());
}

string Identity::id() const throw()
{
	stringstream conv;
	conv << hex << setfill('0') << setw(8) << _id;
	return(conv.str());
}

string Identity::ordinal() const throw()
{
	stringstream	conv;
	conv << hex << setfill('0') << setw(2) << _ordinal;
	return(conv.str());
}

string Identity::shortname() const throw()
{
	return(_shortname);
}

string Identity::longname() const throw()
{
	return(_longname);
}

string Identity::path() const throw()
{
	string				rv;
	string::iterator	it;

	rv	= _parent_path;
	it	= rv.begin() + (rv.length() - 1);

	if(*it == '/')
		rv.erase(it);

	if(rv.empty())
		return(_shortname);

	return(rv + "/" + _shortname);
}

void Identity::_set_longname(string in) throw()
{
	_longname = in;
}

void Identity::_set_shortname(string in) throw()
{
	_shortname = in;
}
