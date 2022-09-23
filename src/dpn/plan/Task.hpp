#ifndef HEADER_dpn_plan_Task_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Task_hpp_ALREADY_INCLUDED

#include <dpn/plan/types.hpp>

#include <string>
#include <ostream>
#include <optional>
#include <set>

namespace dpn { namespace plan { 

	struct Task
	{
		Id id{};
		double effort;
		std::string desc;
		bool is_feature = false;

		Deps ss;
		Deps ff;
		Deps fs;

		std::optional<double> start;
		std::optional<double> stop;

		bool started() const {return !!start;}
		bool stopped() const {return !!stop;}
	};

	std::ostream &operator<<(std::ostream &, const Task &);

} } 

#endif