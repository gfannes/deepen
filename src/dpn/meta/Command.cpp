#include <dpn/meta/Command.hpp>
#include <dpn/log.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 
	
	bool parse(std::optional<Command> &command, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		const auto sp = strange;

		if (strange.pop_if('&'))
		{
			command.emplace();

			gubg::Strange substr;
			auto pop_args = [&](){
				for (gubg::Strange arg; substr.pop_until(arg, ',') || substr.pop_all(arg); )
				{
					arg.strip_left(' ');
					command->arguments.push_back(arg.str());
				}
				return true;
			};

			if (strange.pop_if("include") && strange.pop_bracket(substr, "()"))
			{
				command->type = Command::Include;
				MSS(pop_args());
			}
			else if (strange.pop_if("require") && strange.pop_bracket(substr, "()"))
			{
				command->type = Command::Require;
				MSS(pop_args());
			}
			else
			{
				MSS(false, log::error() << "Could not parse Command from `" << sp.str() << "`" << std::endl);
			}
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Command &command)
	{
		switch (command.type)
		{
			case Command::Invalid: os << "[Invalid]"; break;
			case Command::Include: os << "[Include]"; break;
			case Command::Require: os << "[Require]"; break;
		}
		for (const auto &arg: command.arguments)
			os << "(" << arg << ")";
		return os;
	}

} } 