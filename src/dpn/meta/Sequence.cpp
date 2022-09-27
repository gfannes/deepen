#include <dpn/meta/Sequence.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	void Sequence::merge(const Sequence &other)
	{
		auto m = [](auto &dst, const auto &src){
			if (!dst)
				dst = src;
		};
		m(childs_are_parallel, other.childs_are_parallel);
	}

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
				sequence_opt.emplace().childs_are_parallel = true;
			else if (strange.pop_if('n'))
				sequence_opt.emplace().childs_are_parallel = false;
			else if (strange.pop_if('x'))
				sequence_opt.emplace().fs = false;

			MSS(!!sequence_opt, log::error() << "Could not parse Sequence from " << sp.str() << std::endl);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Sequence &sequence)
	{
		os << "[Sequence]";
		if (!!sequence.childs_are_parallel)
			os << "(childs_are_parallel:" << *sequence.childs_are_parallel << ")";
		if (!!sequence.fs)
			os << "(fs:" << *sequence.fs << ")";
		return os;
	}

} } 