#ifndef HEADER_dpn_meta_Urgency_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Urgency_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Urgency
	{
	public:
		std::optional<unsigned long> reach;
		char impact = 'a';
		std::optional<double> confidence;

		double value() const;

		void merge(const Urgency &rhs);
	};

	bool parse(std::optional<Urgency> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Urgency &);

	template <>
	struct Traits<Urgency>
	{
		static const char * type_name() {return "Urgency";}
	};

} } 

#endif