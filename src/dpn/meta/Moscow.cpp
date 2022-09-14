#include <dpn/meta/Moscow.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool Moscow::any() const
	{
		return must || should || could || wont;
	}

	Moscow Moscow::intersect(const Moscow &rhs) const
	{
		Moscow res;
		res.must = (must && rhs.must);
		res.should = (should && rhs.should);
		res.could = (could && rhs.could);
		res.wont = (wont && rhs.wont);
		return res;
	}

	void Moscow::merge(const Moscow &rhs)
	{
		must = (must || rhs.must);
		should = (should || rhs.should);
		could = (could || rhs.could);
		wont = (wont || rhs.wont);
	}

	bool parse(std::optional<Moscow> &moscow_opt, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		auto sp = strange;

		if (strange.pop_if('.'))
		{
			if (strange.pop_if_any("?@>!.~"))
			{
				// This is a State iso Moscow
				strange = sp;
				return true;
			}
			if (strange.pop_if_any("an"))
			{
				// This is a Sequence iso Moscow
				strange = sp;
				return true;
			}

			if (strange.pop_if('m'))
				moscow_opt.emplace().must = true;
			else if (strange.pop_if('s'))
				moscow_opt.emplace().should = true;
			else if (strange.pop_if('c'))
				moscow_opt.emplace().could = true;
			else if (strange.pop_if('w'))
				moscow_opt.emplace().wont = true;

			MSS(!!moscow_opt, log::error() << "Could not parse Moscow from " << sp.str() << std::endl);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Moscow &moscow)
	{
		os << "[Moscow]";
		if (moscow.must)
			os << "(must)";
		if (moscow.should)
			os << "(should)";
		if (moscow.could)
			os << "(could)";
		if (moscow.wont)
			os << "(wont)";
		return os;
	}

} } 