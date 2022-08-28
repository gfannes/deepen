#include <dpn/meta/Cost.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Cost> &cost, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('$'))
		{
			cost.emplace();
			MSS(strange.pop_float(cost->value));
			MSS(strange.pop_if(' ') || strange.empty());
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Cost &cost)
	{
		os << "[Cost](" << cost.value << ")";
		return os;
	}

} } 