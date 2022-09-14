#ifndef HEADER_dpn_meta_Sequence_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Sequence_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Sequence
	{
	public:
		bool any = false;
	};

	bool parse(std::optional<Sequence> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Sequence &);

	template <>
	struct Traits<Sequence>
	{
		static const char * type_name() {return "Sequence";}
	};

} } 


#endif