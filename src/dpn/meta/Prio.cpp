#include <dpn/meta/Prio.hpp>

#include <gubg/mss.hpp>

#include <cmath>

namespace dpn { namespace meta { 

	double Prio::value() const
	{
		double v = 1.0;

		v *= reach.value_or(1);

		if ('A' <= impact && impact <= 'Z')
		{
			v *= std::exp2(double(impact-'A'));
		}
		else if ('a' <= impact && impact <= 'z')
		{
			v *= 0.75*std::exp2(double(impact-'a'));
		}

		v *= confidence.value_or(1.0);

		return v;
	}

	bool parse(std::optional<Prio> &prio, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('!'))
		{
			prio.emplace();
			{
				unsigned long reach;
				if (strange.pop_decimal(reach))
					prio->reach = reach;
			}
			MSS(strange.pop_char(prio->impact));
			{
				double confidence;
				if (strange.pop_float(confidence))
					prio->confidence = confidence;
			}
			MSS(strange.pop_if(' ') || strange.empty());
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Prio &prio)
	{
		os << "[Prio]";
		if (prio.reach)
			os << "(" << *prio.reach << ")";
		os << "(" << prio.impact << ")";
		if (prio.confidence)
			os << "(" << *prio.confidence << ")";
		os << "(" << prio.value() << ")";
		return os;
	}
} } 