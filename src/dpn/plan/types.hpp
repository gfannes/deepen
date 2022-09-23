#ifndef HEADER_dpn_types_hpp_ALREADY_INCLUDED
#define HEADER_dpn_types_hpp_ALREADY_INCLUDED

#include <map>
#include <set>

namespace dpn { 

	using Id = std::size_t;

	using Id__Id = std::map<Id, Id>;

	using Deps = std::set<Id>;

	using Id__DepIds = std::map<Id, Deps>;

} 

#endif