#ifndef _identify_h_
#define _identify_h_

#include "string"
using std::string;

class Identity
{
	public:

		Identity(int generation, int parent_id, int ordinal, string parent_path)	throw();

		string	generation()	const	throw();
		string	ordinal()		const	throw();
		string	parent_id()		const	throw();
		string	id()			const	throw();
		string	shortname()		const	throw();
		string	longname()		const	throw();
		string	path()			const	throw();

	protected:

		int		_generation;
		int		_ordinal;
		int		_parent_id;
		int		_id;
		string	_shortname;
		string	_longname;
		string	_parent_path;

		void	_set_longname(string)	throw();
		void	_set_shortname(string)	throw();

	private:
};

#endif
