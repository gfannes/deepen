#ifndef HEADER_dpn_meta_Moscow_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Moscow_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Moscow
	{
	public:
		bool must = false;
		bool should = false;
		bool could = false;
		bool wont = false;

		bool any() const;

		void merge(const Moscow &rhs);

		Moscow intersect(const Moscow &rhs) const;
	};

	bool parse(std::optional<Moscow> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Moscow &);

	template <>
	struct Traits<Moscow>
	{
		static const char * type_name() {return "Moscow";}
	};

} } 

#endif