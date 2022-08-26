#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/onto/Node.hpp>

#include <string>
#include <ostream>

namespace dpn { 

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		onto::Nodes nodes;

		bool parse(const std::string &str, Format);

	private:
	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} 

#endif