#ifndef HEADER_dpn_Attribute_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Attribute_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>

namespace dpn { 

	class Attribute
	{
	public:
		std::string key;
		std::string value;

		Attribute() {}
		Attribute(const std::string &key, const std::string &value): key(key), value(value) {}
	};

	using Attributes = std::vector<Attribute>;

} 

#endif