#include <dpn/onto/Library.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

namespace dpn { namespace onto { 

	bool Library::add_file(const std::string &fp)
	{
		MSS_BEGIN(bool);

		std::string content;
		MSS(gubg::file::read(content, fp));

		MSS(!fp__file_.count(fp));
		auto &file = fp__file_[fp];

		MSS(file.parse(content));

		MSS_END();
	}

} } 