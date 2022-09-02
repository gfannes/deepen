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
			if (strange.pop_if("include") && strange.pop_bracket(substr, "()"))
			{
				command->type = Command::Include;
				command->argument = substr.str();
			}
			else if (strange.pop_if("uses") && strange.pop_bracket(substr, "()"))
			{
				command->type = Command::Uses;
				command->argument = substr.str();
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
			case Command::Include: os << "[Include](" << command.argument << ")"; break;
		}
		return os;
	}

} } 