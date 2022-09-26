#ifndef HEADER_dpn_types_hpp_ALREADY_INCLUDED
#define HEADER_dpn_types_hpp_ALREADY_INCLUDED

#include <string>
#include <map>
#include <set>

namespace dpn { 

	using Tags = std::map<std::string, std::string>;
	
	using TagSets = std::map<std::string, std::set<std::string>>;

} 

#endif