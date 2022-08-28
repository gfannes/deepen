#ifndef HEADER_dpn_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Library_hpp_ALREADY_INCLUDED

#include <dpn/File.hpp>

#include <gubg/std/filesystem.hpp>

#include <map>

namespace dpn { 

	class Library
	{
	public:
		void clear();

		bool add_file(const std::filesystem::path &fp);

		bool resolve();

	private:
		bool resolve_include_(std::filesystem::path &fp, const std::string &incl, const std::filesystem::path &context_fp) const;

		std::map<std::filesystem::path, File> fp__file_;
	};

} 

#endif