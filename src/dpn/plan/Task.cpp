#include <dpn/plan/Task.hpp>

#include <gubg/naft/Document.hpp>

namespace dpn { namespace plan { 

	std::ostream &operator<<(std::ostream &os, const Task &task)
	{
		gubg::naft::Document doc{os};
		auto n = doc.node("Task");
		n.attr("id", task.id).attr("effort", task.effort).attr("desc", task.desc);
		if (task.is_feature)
			n.attr("feature");

		auto to_days = [](double q){
			const auto hours = q/4;
			const auto days = hours/8;
			const auto weeks = days/5;
			return weeks;
		};
		if (task.start)
			n.attr("start", to_days(*task.start));
		if (task.stop)
			n.attr("stop", to_days(*task.stop));
		return os;
	}

} } 