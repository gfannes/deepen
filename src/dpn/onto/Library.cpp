#include <dpn/onto/Library.hpp>
#include <dpn/log.hpp>

#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

#include <optional>

namespace dpn { namespace onto { 
	void Library::clear()
	{
		*this = Library{};
	}

	bool Library::add_file(const std::filesystem::path &fp)
	{
		MSS_BEGIN(bool);

		std::string content;
		MSS(gubg::file::read(content, fp));

		MSS(!fp__file_.count(fp));
		auto &file = fp__file_[fp];

		std::optional<File::Format> format;
		{
			const auto extension = fp.extension().string();
			if (extension == ".md") format = File::Format::Markdown;
			else if (extension == ".naft") format = File::Format::Naft;
			else if (extension == ".mm") format = File::Format::Freemind;
			MSS(!!format, log::error() << "Could not determin format from extension " << extension << std::endl);
		}

		MSS(file.parse(content, *format));

		MSS_END();
	}

} } 