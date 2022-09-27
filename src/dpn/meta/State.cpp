#include <dpn/meta/State.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool State::operator==(const State &rhs) const
	{
		if (status != rhs.status)
			return false;
		if (text != rhs.text)
			return false;
		return true;
	}

	bool parse(std::optional<State> &state_opt, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		auto sp = strange;

		if (strange.pop_if('.'))
		{
			if (strange.pop_if_any("mscw"))
			{
				// This is a Moscow iso State
				strange = sp;
				return true;
			}
			if (strange.pop_if_any("anx"))
			{
				// This is a Sequence iso State
				strange = sp;
				return true;
			}

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

	std::string to_string(Status status)
	{
		switch (status)
		{
			case Status::Inbox: return "Inbox"; break;
			case Status::Actionable: return "Actionable"; break;
			case Status::Forwarded: return "Forwarded"; break;
			case Status::WIP: return "WIP"; break;
			case Status::Done: return "Done"; break;
			case Status::Canceled: return "Canceled"; break;
		}
		return "<Unknown status>";
	}

	std::ostream &operator<<(std::ostream &os, const State &state)
	{
		return os << state.status;
	}

} } 