#ifndef HEADER_dpn_plan_Graph_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Graph_hpp_ALREADY_INCLUDED

#include <dpn/plan/types.hpp>

#include <map>

namespace dpn { namespace plan { 

	class Graph
	{
	public:
		Ids vertices;
		Id__Id parent;
		Id__Ids fs;
		Id__Ids ff;
		Id__Ids ss;

		void clear() {*this = Graph{};}
	};

} } 

#endif