#ifndef HEADER_dpn_plan_Graph_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Graph_hpp_ALREADY_INCLUDED

#include <dpn/plan/types.hpp>

#include <map>
#include <ostream>

namespace dpn { namespace plan { 

	class Graph
	{
	public:
		Ids vertices;
		std::map<Id, std::string> text;
		std::map<Id, unsigned int> my_effort;
		std::map<Id, unsigned int> agg_effort;
		std::map<Id, unsigned int> depth;
		Id__Id parent;
		Id__Ids fs;
		Id__Ids ff;
		Id__Ids ss;

		void clear() {*this = Graph{};}

		void erase(Id id)
		{
			vertices.erase(id);
			text.erase(id);
			my_effort.erase(id);
			agg_effort.erase(id);
			depth.erase(id);
			parent.erase(id);
			fs.erase(id);
			ff.erase(id);
			ss.erase(id);
		}
	};

	std::ostream &operator<<(std::ostream &, const Graph &);

} } 

#endif