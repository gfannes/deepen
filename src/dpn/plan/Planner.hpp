#ifndef HEADER_dpn_plan_Planner_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Planner_hpp_ALREADY_INCLUDED

#include <dpn/plan/types.hpp>
#include <dpn/plan/Resources.hpp>
#include <dpn/plan/Task.hpp>
#include <dpn/plan/Plan.hpp>
#include <dpn/List.hpp>

#include <string>
#include <vector>
#include <map>
#include <set>

namespace dpn { namespace plan { 

	class Planner
	{
	public:
		bool load_resources(const std::string &filepath);
		bool setup_tasks(const List &nodes, const Id__Id &part_of, const Id__Id &after, const Id__DepIds &reqs);

		bool operator()(Plan &);

	private:
		std::vector<Task> tasks_;

		Resources resources_;
	};

} } 

#endif
