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

	void Urgency::merge(const Urgency &rhs)
	{
		if (rhs.reach)
			reach = std::max(reach.value_or(1), *rhs.reach);

		auto to_ix = [](char ch){
			if ('a' <= ch && ch <= 'z')
			{
				const unsigned int ix = ch-'a';
				return 2*ix;
			}
			if ('A' <= ch && ch <= 'Z')
			{
				const unsigned int ix = ch-'A';
				return 2*ix+1;
			}
			return 0u;
		};
		auto from_ix = [](unsigned int ix) -> char{
			if (ix%2 == 0)
			{
				ix /= 2;
				if (ix < 26)
					return 'a'+ix;
			}
			else
			{
				ix /= 2;
				if (ix < 26)
					return 'A'+ix;
			}
			return '\0';
		};

		const auto my_ix = to_ix(impact);
		const auto rhs_ix = to_ix(rhs.impact);
		const auto max_ix = std::max(my_ix, rhs_ix);
		impact = from_ix(max_ix);

		if (rhs.confidence)
			confidence = std::max(confidence.value_or(1.0), *rhs.confidence);
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