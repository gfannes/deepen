#include <dpn/meta/Effort.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	std::string Effort::str() const
	{
		auto minutes = total_minutes;

		auto hours = minutes/60;
		minutes = minutes%60;

		auto days = hours/8;
		hours = hours%8;

		auto weeks = days/5;
		days = days%5;

		std::string s;
		if (weeks > 0)
			s += std::to_string(weeks) + "w";
		if (days > 0)
			s += std::to_string(days) + "d";
		if (hours > 0)
			s += std::to_string(hours) + "h";
		if (minutes > 0)
			s += std::to_string(minutes) + "m";

		if (s.empty())
			s = "0m";

		return s;
	}

	Effort &Effort::operator+=(const Effort &rhs)
	{
		total_minutes += rhs.total_minutes;
		return *this;
	}

	bool parse(std::optional<Effort> &effort, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('$'))
		{
			gubg::Strange substr;
			MSS(strange.pop_until(substr, ' ') || strange.pop_all(substr));
			auto sp = substr;
			effort.emplace();

			unsigned int weeks = 0, days = 0, hours = 0, minutes = 0;
			{
				gubg::Strange tmp;
				if (substr.pop_until(tmp, 'w'))
					tmp.pop_decimal(weeks);
				if (substr.pop_until(tmp, 'd'))
					tmp.pop_decimal(days);
				if (substr.pop_until(tmp, 'h'))
					tmp.pop_decimal(hours);
				if (substr.pop_until(tmp, 'm'))
					tmp.pop_decimal(minutes);
			}
			MSS(substr.empty(), log::error() << "Could not parse effort from '" << sp.str() << "'" << std::endl);

			days += weeks*5;
			hours += days*8;
			minutes += hours*60;

			effort->total_minutes = minutes;
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Effort &effort)
	{
		os << "[Effort](" << effort.str() << ")";
		return os;
	}

} } 