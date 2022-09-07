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
		unsigned int total = 0;
		unsigned int done = 0;

		unsigned int todo() const {return total-done;}
		std::optional<unsigned int> completion() const;

		std::string str() const;

		Effort &operator+=(const Effort &rhs);

		static std::string to_dsl(unsigned int count);
		static bool from_dsl(unsigned int &count, gubg::Strange &);
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