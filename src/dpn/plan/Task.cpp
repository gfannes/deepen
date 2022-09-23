#include <dpn/plan/Task.hpp>

#include <gubg/naft/Document.hpp>

namespace dpn { namespace plan { 

	std::ostream &operator<<(std::ostream &os, const Task &task)
	{
		gubg::naft::Document doc{os};
		doc.node("Task").attr("id", task.id).attr("effort", task.effort).attr("desc", task.desc);
		return os;
	}

} } 