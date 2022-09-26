#ifndef HEADER_dpn_meta_Tag_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Tag_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>

#include <string>
#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Tag
	{
	public:
		using Opt = std::optional<Tag>;

		std::string key;
		std::string value;
	};

	bool parse(Tag::Opt &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Tag &);

} } 

#endif