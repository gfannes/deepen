#include <dpn/meta/Sequence.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Sequence> &sequence_opt, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		auto sp = strange;

		if (strange.pop_if('.'))
		{
			if (strange.pop_if_any("?@>!.~"))
			{
				// This is a State iso Sequence
				strange = sp;
				return true;
			}
			if (strange.pop_if_any("mscw"))
			{
				// This is a Moscow iso Sequence
				strange = sp;
				return true;
			}

			if (strange.pop_if('a'))
				sequence_opt.emplace().any = true;
			else if (strange.pop_if('n'))
				sequence_opt.emplace().any = false;

			MSS(!!sequence_opt, log::error() << "Could not parse Sequence from " << sp.str() << std::endl);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Sequence &sequence)
	{
		os << "[Sequence](any:" << sequence.any << ")";
		return os;
	}

} } 