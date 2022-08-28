#ifndef HEADER_dpn_meta_Data_hpp_ALREADY_INCLUDED
#define HEADER_dpn_meta_Data_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>

#include <string>
#include <optional>
#include <ostream>

namespace dpn { namespace meta { 

	class Data
	{
	public:
		std::string key;
		std::string value;
	};

	bool parse(std::optional<Data> &, gubg::Strange &);

	std::ostream &operator<<(std::ostream &, const Data &);

} } 

#endif