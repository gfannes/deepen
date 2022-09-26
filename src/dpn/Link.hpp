#ifndef HEADER_dpn_Link_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Link_hpp_ALREADY_INCLUDED

#include <dpn/Filter.hpp>

#include <filesystem>
#include <vector>

namespace dpn { 

	class Link
	{
	public:
		enum Type {Include, Require};

		Type type = Type::Include;
		std::filesystem::path fp;
		Filter filter;
	};

	using Links = std::vector<Link>;
	Link &goc(Links &, Link::Type, const std::filesystem::path &);

} 

#endif