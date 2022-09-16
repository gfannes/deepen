#ifndef HEADER_dpn_meta_Command_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Command_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <string>
#include <vector>
#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Command
	{
	public:
		enum Type {Invalid, Include, Require};

		Type type = Invalid;
		std::vector<std::string> arguments;

		Command() {}
		Command(Type type, const std::string &argument): type(type) {arguments.push_back(argument);}
	};

	bool parse(std::optional<Command> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &os, const Command &command);

	template <>
	struct Traits<Command>
	{
		static const char * type_name() {return "Command";}
	};

} } 

#endif