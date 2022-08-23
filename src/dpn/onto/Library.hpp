#ifndef HEADER_dpn_onto_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Library_hpp_ALREADY_INCLUDED

#include <dpn/onto/File.hpp>
#include <string>
#include <map>

namespace dpn { namespace onto { 

	class Library
	{
	public:
		bool add_file(const std::string &fp);

	private:
		std::map<std::string, File> fp__file_;
	};

} } 

#endif