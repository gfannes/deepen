#ifndef HEADER_dpn_onto_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_File_hpp_ALREADY_INCLUDED

#include <string>

namespace dpn { namespace onto { 

	class File
	{
	public:
		bool parse(const std::string &str);
	};

} } 

#endif