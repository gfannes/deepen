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
		MSS(gubg::file::read(content, fp), log::error() << "Could not read content from " << fp << std::endl);

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

				MSS(file.interpret(), log::error() << "Could not interpret " << fp << std::endl);

				std::vector<std::string> failures;
				file.each_node([&](auto &n, const auto &path){
					for (const auto &meta: n.metas)
					{
						if (auto *command = std::get_if<meta::Command>(&meta))
						{
							if (command->type == meta::Command::Include)
							{
								const auto &include = command->argument;
								std::filesystem::path incl_fp;
								if (!resolve_include_(incl_fp, include, fp))
								{
									failures.push_back(include);
									continue;
								}
								n.includes.insert(incl_fp);
								for (const auto &child: n.childs)
									n.includes.insert(child.includes.begin(), child.includes.end());
								file.includes.insert(incl_fp);
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
					}
				}, Direction::Pull);
				MSS(failures.empty());

				done.insert(fp);
			}
		}

		// Keep merging file.includes until things stabilize
		for (bool dirty = true; dirty; )
		{
			dirty = false;
			for (auto &[_,file]: fp__file_)
			{
				const auto orig_size = file.includes.size();

				for (const auto &incl_fp: file.includes)
				{
					auto it = fp__file_.find(incl_fp);
					MSS(it != fp__file_.end());
					const auto &incl_file = it->second;
					file.includes.insert(incl_file.includes.begin(), incl_file.includes.end());
				}

				const auto new_size = file.includes.size();

				if (orig_size != new_size)
					dirty = true;
			}
		}

		// Update the Node.includes with the info for File
		for (auto &[_,file]: fp__file_)
		{
			auto complete_includes = [&](auto &n, const auto &path){
				const auto initial_includes = n.includes;
				for (const auto &initial_include: initial_includes)
				{
					auto it = fp__file_.find(initial_include);
					if (it != fp__file_.end())
						n.includes.insert(it->second.includes.begin(), it->second.includes.end());
				}
			};
			file.each_node(complete_includes, Direction::Pull);
		}

		for (auto &[_,file]: fp__file_)
		{
			for (const auto &incl_fp: file.includes)
			{
				auto it = fp__file_.find(incl_fp);
				MSS(it != fp__file_.end());
				file.total_effort += it->second.local_effort;
			}
			auto update_total_effort = [&](auto &n, const auto &path){
				n.total_effort = n.local_effort;
				for (const auto &incl_fp: n.includes)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
						n.total_effort += it->second.local_effort;
				}
			};
			file.each_node(update_total_effort, Direction::Push);
		}

		MSS_END();
	}

	void Library::print(std::ostream &os) const
	{
		for (const auto &[fp,file]: fp__file_)
		{
			os << std::endl;
			os << "File " << fp << " " << file.local_effort << " " << file.total_effort << std::endl;
			auto print_node = [&](const auto &n, const auto &path){
				os << std::string(path.size(), ' ');
				for (const auto &meta: n.metas)
				{
					if (auto *state = std::get_if<meta::State>(&meta))
						os << "[State](" << *state << ")";
					else if (auto *effort = std::get_if<meta::Effort>(&meta))
						os << *effort;
					else if (auto *duedate = std::get_if<meta::Duedate>(&meta))
						os << *duedate;
					else if (auto *prio = std::get_if<meta::Prio>(&meta))
						os << *prio;
					else if (auto *command = std::get_if<meta::Command>(&meta))
						os << *command;
					else if (auto *data = std::get_if<meta::Data>(&meta))
						os << *data;
				}
				os << n.my_effort << n.local_effort << n.total_effort;
				os << "[Text](" << n.text << ")" << std::endl;
			};
			file.each_node(print_node, Direction::Push);
		}
	}

	bool Library::get(List &list, meta::State wanted_state) const
	{
		MSS_BEGIN(bool);

		list.clear();

		for (const auto &p: fp__file_)
		{
			const auto &fp = p.first;
			const auto &file = p.second;

			auto append = [&](const auto &n, const auto &path){
				if (auto *state = n.template get<meta::State>(); !!state && *state == wanted_state)
				{
					List::Item item;
					item.text = n.text;
					item.fp = fp;
					// item.state = *state;
					if (auto *ptr = n.template get<meta::Prio>())
						item.prio = ptr->value();
					if (auto *ptr = n.template get<meta::Effort>())
						item.total_minutes = ptr->total_minutes;
					list.items.emplace_back(item);
				}
			};
			file.each_node(append, Direction::Push);
		}

		MSS_END();
	}

	bool Library::get_due(List &list) const
	{
		MSS_BEGIN(bool);

		list.clear();

		for (const auto &p: fp__file_)
		{
			const auto &fp = p.first;
			const auto &file = p.second;

			auto append = [&](const auto &n, const auto &path){
				if (auto *duedate = n.template get<meta::Duedate>())
				{
					List::Item item;
					item.text = n.text;
					item.fp = fp;
					item.yyyymmdd = duedate->yyyymmdd();
					if (auto *ptr = n.template get<meta::State>())
						item.state = *ptr;
					if (auto *ptr = n.template get<meta::Prio>())
						item.prio = ptr->value();
					if (auto *ptr = n.template get<meta::Effort>())
						item.total_minutes = ptr->total_minutes;
					list.items.emplace_back(item);
				}
			};
			file.each_node(append, Direction::Push);
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