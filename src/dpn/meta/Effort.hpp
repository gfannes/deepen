#ifndef HEADER_dpn_meta_Effort_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Effort_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Effort
	{
	public:
		unsigned int total_minutes = 0;

		std::string str() const;

		Effort &operator+=(const Effort &rhs);
	};

	bool parse(std::optional<Effort> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Effort &);

	template <>
	struct Traits<Effort>
	{
		static const char * type_name() {return "Effort";}
	};

} } 

#endif