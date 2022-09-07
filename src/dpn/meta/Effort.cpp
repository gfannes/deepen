#include <dpn/meta/Effort.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	std::string Effort::to_dsl(unsigned int count)
	{
		auto quarters = count;

		auto hours = quarters/4;
		quarters = quarters%4;

		auto days = hours/8;
		hours = hours%8;

		auto weeks = days/5;
		days = days%5;

		auto months = weeks/4;
		weeks = weeks%4;

		std::string s;
		if (months > 0)
			s += std::to_string(months) + "m";
		if (weeks > 0)
			s += std::to_string(weeks) + "w";
		if (days > 0)
			s += std::to_string(days) + "d";
		if (hours > 0)
			s += std::to_string(hours) + "h";
		if (quarters > 0)
			s += std::to_string(quarters) + "q";

		if (s.empty())
			s = "0h";

		return s;
	}

	bool Effort::from_dsl(unsigned int &count, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		auto sp = strange;

		unsigned int months = 0, weeks = 0, days = 0, hours = 0, quarters = 0;
		{
			gubg::Strange tmp;
			if (strange.pop_until(tmp, 'm'))
				tmp.pop_decimal(months);
			if (strange.pop_until(tmp, 'w'))
				tmp.pop_decimal(weeks);
			if (strange.pop_until(tmp, 'd'))
				tmp.pop_decimal(days);
			if (strange.pop_until(tmp, 'h'))
				tmp.pop_decimal(hours);
			if (strange.pop_until(tmp, 'q'))
				tmp.pop_decimal(quarters);
		}
		MSS(strange.empty(), log::error() << "Could not parse effort from '" << sp.str() << "'" << std::endl);

		weeks += months*4;
		days += weeks*5;
		hours += days*8;
		quarters += hours*4;

		count = quarters;

		MSS_END();
	}

	std::optional<unsigned int> Effort::completion() const
	{
		std::optional<unsigned int> res;
		if (total > 0)
			res = (100*done)/total;
		return res;
	}

	std::string Effort::str() const
	{
		std::string s;
		s = to_dsl(todo())+"/"+to_dsl(total);
		if (auto c = completion())
			s += "/" + std::to_string(*c) + "%";
		return s;
	}

	Effort &Effort::operator+=(const Effort &rhs)
	{
		total += rhs.total;
		done += rhs.done;
		return *this;
	}

	bool parse(std::optional<Effort> &effort, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('$'))
		{
			gubg::Strange substr;
			MSS(strange.pop_until(substr, ' ') || strange.pop_all(substr));

			effort.emplace();
			MSS(Effort::from_dsl(effort->total, substr));
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Effort &effort)
	{
		os << effort.str();
		return os;
	}

} } 