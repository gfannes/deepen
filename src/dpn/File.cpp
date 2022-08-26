#include <dpn/File.hpp>
#include <dpn/log.hpp>
#include <dpn/read/markdown.hpp>

#include <gubg/mss.hpp>

namespace dpn { 

	bool File::parse(const std::string &str, Format format)
	{
		MSS_BEGIN(bool);

		switch (format)
		{
			case Format::Markdown: MSS(read::markdown(nodes, str)); break;
			case Format::Naft: MSS(read::naft(nodes, str)); break;
			case Format::Freemind: MSS(read::freemind(nodes, str)); break;

			default: MSS(false, log::error() << "Format " << format << " not supported" << std::endl); break;
		}

		MSS_END();
	}

	// Free functions
	std::ostream &operator<<(std::ostream &os, File::Format format)
	{
		switch (format)
		{
			case File::Format::Markdown: os << "Markdown"; break;
			case File::Format::Naft: os << "Naft"; break;
			case File::Format::Freemind: os << "Freemind"; break;
		}
		return os;
	}

	// Privates

} 