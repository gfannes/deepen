#ifndef HEADER_dpn_plan_Resources_hpp_ALREADY_INCLUDED
#define HEADER_dpn_plan_Resources_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>
#include <vector>

namespace dpn { namespace plan { 

	struct Resource
	{
		std::string name;
		double capacity =  0.0;
	};

	class Resources: public std::vector<Resource>
	{
	public:
        bool load_from_file(const std::string &filepath);
	};

	std::ostream &operator<<(std::ostream &, const Resources &);

} } 

#endif