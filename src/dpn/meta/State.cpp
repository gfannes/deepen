#include <dpn/meta/State.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace meta { 

	bool parse(std::optional<State> &state, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('.'))
		{
			auto check = [&](char ch, State s){
				MSS_BEGIN(bool);
				if (!state && strange.pop_if(ch))
				{
					MSS(strange.pop_if(' ') || strange.empty());
					state = s;
				}
				MSS_END();
			};

			MSS(check('?', State::Inbox));
			MSS(check('@', State::Actionable));
			MSS(check('>', State::Forwarded));
			MSS(check('!', State::WIP));
			MSS(check('.', State::Done));
			MSS(check('~', State::Canceled));

			MSS(!!state);
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, State state)
	{
		switch (state)
		{
			case State::Inbox: os << "Inbox"; break;
			case State::Actionable: os << "Actionable"; break;
			case State::Forwarded: os << "Forwarded"; break;
			case State::WIP: os << "WIP"; break;
			case State::Done: os << "Done"; break;
			case State::Canceled: os << "Canceled"; break;
		}
		return os;
	}

} } 