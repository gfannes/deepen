#include <dpn/onto/File.hpp>
#include <dpn/log.hpp>

#include <gubg/markdown/Reader.hpp>
#include <gubg/xml/Reader.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace onto { 

	bool File::parse(const std::string &str, Format format)
	{
		MSS_BEGIN(bool);

		switch (format)
		{
			case Format::Markdown: MSS(parse_markdown_(str)); break;
			case Format::Naft: MSS(parse_naft_(str)); break;
			case Format::Freemind: MSS(parse_freemind_(str)); break;

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
	bool File::parse_markdown_(const std::string &str)
	{
		MSS_BEGIN(bool);
		using namespace gubg;

		log::os(0) << "Parsing markdown from " << str << std::endl;

		markdown::Reader reader{str};
		for (markdown::Reader::Item item; reader(item); )
		{
		}
		MSS(!reader.error, log::error() << "Could not parse markdown due to: " << *reader.error << std::endl);

		MSS_END();
	}

	bool File::parse_naft_(const std::string &str)
	{
		MSS_BEGIN(bool);

		MSS(false, log::internal_error() << "Not implemented" << std::endl);

		MSS_END();
	}

	bool File::parse_freemind_(const std::string &str)
	{
		MSS_BEGIN(bool);

		MSS(false, log::internal_error() << "Not implemented" << std::endl);

		MSS_END();
	}


} } 