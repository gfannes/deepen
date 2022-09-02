#ifndef HEADER_dpn_meta_Command_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Command_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <string>
#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Command
	{
	public:
		enum Type {Invalid, Include, Uses};

		Type type = Invalid;
		std::string argument;

		Command() {}
		Command(Type type, const std::string &argument): type(type), argument(argument) {}
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