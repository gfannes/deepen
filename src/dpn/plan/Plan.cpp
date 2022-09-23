#include <dpn/plan/Plan.hpp>

#include <iostream>

namespace dpn { namespace plan { 

	void Plan::operator()(const Task &task)
	{
		// if (task.is_feature)
			std::cout << task;
	}

	std::ostream &operator<<(std::ostream &os, const Plan &plan)
	{
		return os;
	}

} } 