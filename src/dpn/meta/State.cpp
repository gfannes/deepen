#include <dpn/meta/State.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<State> &state_opt, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('.'))
		{
			auto check = [&](char ch, Status status){
				MSS_BEGIN(bool);
				if (!state_opt && strange.pop_if(ch))
				{
					auto &state = state_opt.emplace();
					state.status = status;
					strange.pop_until(state.text, ' ') || strange.pop_all(state.text);
				}
				MSS_END();
			};

			MSS(check('?', Status::Inbox));
			MSS(check('@', Status::Actionable));
			MSS(check('>', Status::Forwarded));
			MSS(check('!', Status::WIP));
			MSS(check('.', Status::Done));
			MSS(check('~', Status::Canceled));

			MSS(!!state_opt);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, Status status)
	{
		switch (status)
		{
			case Status::Inbox: os << "Inbox"; break;
			case Status::Actionable: os << "Actionable"; break;
			case Status::Forwarded: os << "Forwarded"; break;
			case Status::WIP: os << "WIP"; break;
			case Status::Done: os << "Done"; break;
			case Status::Canceled: os << "Canceled"; break;
		}
		return os;
	}

	std::ostream &operator<<(std::ostream &os, const State &state)
	{
		return os << state.status;
	}

} } 