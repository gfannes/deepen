#include <dpn/meta/Urgency.hpp>

#include <gubg/mss.hpp>

#include <cmath>

namespace dpn { namespace meta { 

	double Urgency::value() const
	{
		double v = 1.0;

		v *= reach.value_or(1);

		if ('a' <= impact && impact <= 'z')
		{
			v *= std::exp2(double(impact-'a'));
		}
		else if ('A' <= impact && impact <= 'Z')
		{
			v *= 1.5*std::exp2(double(impact-'A'));
		}

		v *= confidence.value_or(1.0);

		return v;
	}

	bool parse(std::optional<Urgency> &urgency, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);
		L(C(strange.str()));

		if (strange.pop_if('!'))
		{
			gubg::Strange substr;
			MSS(strange.pop_until(substr, ' ') || strange.pop_all(substr));
			urgency.emplace();
			{
				unsigned long reach;
				if (substr.pop_decimal(reach))
					urgency->reach = reach;
			}
			MSS(substr.pop_char(urgency->impact));
			{
				double confidence;
				if (substr.pop_float(confidence))
					urgency->confidence = confidence;
			}
			MSS(substr.empty());
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Urgency &urgency)
	{
		os << "[Urgency]";
		if (urgency.reach)
			os << "(" << *urgency.reach << ")";
		os << "(" << urgency.impact << ")";
		if (urgency.confidence)
			os << "(" << *urgency.confidence << ")";
		os << "(" << urgency.value() << ")";
		return os;
	}
} } 