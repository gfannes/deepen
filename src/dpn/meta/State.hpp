#ifndef HEADER_dpn_meta_State_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_State_hpp_ALREADY_INCLUDED

#include <dpn/meta/Traits.hpp>

#include <gubg/Strange.hpp>

#include <optional>
#include <string>
#include <ostream>

namespace dpn { namespace meta { 

	enum class Status
	{
		Inbox,
		Actionable,
		Forwarded,
		WIP,
		Done,
		Canceled,
	};

	std::ostream &operator<<(std::ostream &, Status);

	class State
	{
	public:
		Status status = Status::Inbox;

		std::string text;
	};

	bool parse(std::optional<State> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const State &);

	template <>
	struct Traits<State>
	{
		static const char * type_name() {return "State";}
	};

} } 

#endif