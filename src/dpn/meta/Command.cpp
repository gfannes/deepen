#include <dpn/meta/Command.hpp>

#include <gubg/mss.hpp>

namespace dpn { namespace meta { 
	
	bool parse(std::optional<Command> &command, gubg::Strange &strange)
	{
		MSS_BEGIN(bool);

		if (strange.pop_if('&'))
		{
			command.emplace();

			gubg::Strange substr;
			strange.pop_until(substr, ' ') || strange.pop_all(substr);

			if (substr.pop_if("include:"))
			{
				command->type = Command::Include;
				command->argument = substr.str();
			}
			else
			{
				MSS(false);
			}
		}

		MSS_END();
	}

	std::ostream &operator<<(std::ostream &os, const Command &command)
	{
		switch (command.type)
		{
			case Command::Invalid: os << "[Invalid]"; break;
			case Command::Include: os << "[Include](" << command.argument << ")"; break;
		}
		return os;
	}

} } 