#ifndef HEADER_dpn_meta_State_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_State_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	enum class State
	{
		Inbox,
		Actionable,
		Forwarded,
		WIP,
		Done,
		Canceled,
	};

	bool parse(std::optional<State> &state, gubg::Strange &strange);

	std::ostream &operator<<(std::ostream &os, State state);

	template <>
	struct Traits<State>
	{
		static const char * type_name() {return "State";}
	};

} } 

#endif