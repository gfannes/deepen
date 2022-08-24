#ifndef HEADER_dpn_onto_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_Library_hpp_ALREADY_INCLUDED

#include <dpn/onto/File.hpp>

#include <gubg/std/filesystem.hpp>

#include <map>

namespace dpn { namespace onto { 

	class Library
	{
	public:
		void clear();

		bool add_file(const std::filesystem::path &fp);

	private:
		std::map<std::filesystem::path, File> fp__file_;
	};

} } 

#endif