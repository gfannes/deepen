#ifndef HEADER_dpn_meta_Duedate_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Duedate_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>

namespace dpn { namespace meta { 

	class Duedate
	{
	public:
		unsigned long yyyy = 0;
		unsigned long mm = 0;
		unsigned long dd = 0;

		unsigned long yyyymmdd() const {return yyyy*10000u+mm*100u+dd;}
	};

	bool parse(std::optional<Duedate> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &os, const Duedate &duedate);

	template <>
	struct Traits<Duedate>
	{
		static const char * type_name() {return "Duedate";}
	};

} } 

#endif