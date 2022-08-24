#ifndef HEADER_dpn_onto_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_onto_File_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>

namespace dpn { namespace onto { 

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		bool parse(const std::string &str, Format);

	private:
		bool parse_markdown_(const std::string &str);
		bool parse_naft_(const std::string &str);
		bool parse_freemind_(const std::string &str);
	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} } 

#endif