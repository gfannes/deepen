#ifndef HEADER_dpn_meta_Prio_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Prio_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Prio
	{
	public:
		std::optional<unsigned long> reach;
		char impact = 'A';
		std::optional<double> confidence;

		double value() const;
	};

	bool parse(std::optional<Prio> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Prio &);

	template <>
	struct Traits<Prio>
	{
		static const char * type_name() {return "Prio";}
	};

} } 

#endif