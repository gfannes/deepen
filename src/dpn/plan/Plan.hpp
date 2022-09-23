#ifndef HEADER_dpn_plan_Plan_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Plan_hpp_ALREADY_INCLUDED

#include <dpn/plan/types.hpp>
#include <dpn/plan/Task.hpp>

#include <ostream>
#include <list>

namespace dpn { namespace plan { 

	class Plan
	{
	public:
		void clear() {*this = Plan{};}

		void operator()(const Task &);
	};

	std::ostream &operator<<(std::ostream &, const Plan &);

} } 

#endif