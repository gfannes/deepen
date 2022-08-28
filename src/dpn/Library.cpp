#include <dpn/Library.hpp>
#include <dpn/log.hpp>

#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

#include <optional>
#include <set>

namespace dpn { 
	void Library::clear()
	{
		*this = Library{};
	}

	bool Library::add_file(const std::filesystem::path &fp)
	{
		MSS_BEGIN(bool, "");

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

	bool Library::resolve()
	{
		MSS_BEGIN(bool, "");

		std::set<std::filesystem::path> done;
		auto get_todos = [&](auto &fps_todo){
			fps_todo.clear();
			for (const auto &[fp,_]: fp__file_)
			{
				if (!done.count(fp))
					fps_todo.push_back(fp);
			}
			return !fps_todo.empty();
		};

		for (std::vector<std::filesystem::path> fps_todo; get_todos(fps_todo); )
		{
			S("");
			for (const auto &fp: fps_todo)
			{
				auto &file = fp__file_[fp];

				MSS(file.interpret());

				std::vector<std::string> failures;
				file.each_node([&](const auto &n){
					for (const auto &cmd: n.commands)
					{
						if (cmd.first == "include")
						{
							const auto &include = cmd.second;
							std::filesystem::path incl_fp;
							if (!resolve_include_(incl_fp, include, fp))
							{
								failures.push_back(include);
								continue;
							}
							if (fp__file_.count(incl_fp))
							{
								continue;
							}
							if (!add_file(incl_fp))
							{
								failures.push_back(include);
								continue;
							}
						}
					}
				});
				MSS(failures.empty());

				done.insert(fp);
			}
		}

		MSS_END();
	}

	// Privates
	bool Library::resolve_include_(std::filesystem::path &fp, const std::string &incl, const std::filesystem::path &context_fp) const
	{
		MSS_BEGIN(bool);

		fp = incl;

		if (!fp.is_absolute())
			fp = context_fp.parent_path() / incl;

		MSS_END();
	}

} 