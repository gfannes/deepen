#ifndef HEADER_dpn_meta_Cost_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Cost_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Cost
	{
	public:
		double value = 0.0;
	};

	bool parse(std::optional<Cost> &cost, gubg::Strange &strange);

	std::ostream &operator<<(std::ostream &os, const Cost &cost);

	template <>
	struct Traits<Cost>
	{
		static const char * type_name() {return "Cost";}
	};

} } 

#endif