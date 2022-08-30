#include <dpn/meta/Effort.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Effort> &effort, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('$'))
		{
			effort.emplace();
			MSS(strange.pop_float(effort->hours));
			MSS(strange.pop_if(' ') || strange.empty());
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Effort &effort)
	{
		os << "[Effort](" << effort.hours << ")";
		return os;
	}

} } 