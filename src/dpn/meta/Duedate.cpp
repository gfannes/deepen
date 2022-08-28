#include <dpn/meta/Duedate.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<Duedate> &duedate, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('?'))
		{
			duedate.emplace();
			unsigned long yyyymmdd;
			MSS(strange.pop_decimal(yyyymmdd));
			MSS(strange.pop_if(' ') || strange.empty());

			duedate->dd = yyyymmdd%100u;
			yyyymmdd /= 100u;
			
			duedate->mm = yyyymmdd%100u;
			yyyymmdd /= 100u;

			duedate->yyyy = yyyymmdd;
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Duedate &duedate)
	{
		os << "[Duedate](" << duedate.yyyy << ")(" << duedate.mm << ")(" << duedate.dd << ")";
		return os;
	}

} } 
