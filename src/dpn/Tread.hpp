#ifndef HEADER_dpn_Tread_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Tread_hpp_ALREADY_INCLUDED

#include <dpn/enums.hpp>

namespace dpn { 

	// Indicates how we should walk through the Nodes
	struct Tread
	{
		Direction direction = Direction::Push;
		Dependency dependency = Dependency::None;
		bool include_link_nodes = false;
	};

} 

#endif