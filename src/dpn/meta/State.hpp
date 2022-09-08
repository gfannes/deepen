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

	std::string to_string(Status);
	inline std::ostream &operator<<(std::ostream &os, Status status) {return os << to_string(status);}

	class State
	{
	public:
		Status status = Status::Inbox;

		std::string text;

		bool operator==(const State &rhs) const;
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