#include <dpn/Library.hpp>
#include <dpn/log.hpp>

#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

#include <optional>
#include <set>

namespace dpn { 
	void Library::clear()
	{
		fp__file_.clear();
	}

	bool Library::add_file(const std::filesystem::path &fp)
	{
		MSS_BEGIN(bool);

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
		MSS_BEGIN(bool);

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
					// Merge the includes of all the childs into our own includes. This relies on Direction::Pull
					for (const auto &child: n.childs)
						n.includes.insert(child.includes.begin(), child.includes.end());
				}, Direction::Pull);
				MSS(failures.empty());

				done.insert(fp);
			}
		}

		// Keep merging file.root.includes until things stabilize
		for (bool dirty = true; dirty; )
		{
			dirty = false;
			for (auto &[_,file]: fp__file_)
			{
				const auto orig_size = file.root.includes.size();

				for (const auto &incl_fp: file.root.includes)
				{
					auto it = fp__file_.find(incl_fp);
					MSS(it != fp__file_.end());
					const auto &incl_file = it->second;
					file.root.includes.insert(incl_file.root.includes.begin(), incl_file.root.includes.end());
				}

				const auto new_size = file.root.includes.size();

				if (orig_size != new_size)
					dirty = true;
			}
		}

		// Update the Node.includes with the info for File.root
		for (auto &[_,file]: fp__file_)
		{
			auto complete_includes = [&](auto &n, const auto &path){
				const auto initial_includes = n.includes;
				for (const auto &initial_include: initial_includes)
				{
					auto it = fp__file_.find(initial_include);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						n.includes.insert(file.root.includes.begin(), file.root.includes.end());
					}
				}
			};
			file.each_node(complete_includes, Direction::Pull);
		}

		// Compute the total_effort as local_effort + local_effort of all includes
		// The Direction does not matter
		for (auto &[_,file]: fp__file_)
		{
			auto compute_total_effort = [&](auto &node, const auto &path){
				node.total_effort = node.local_effort;
				for (const auto &incl_fp: node.includes)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						node.total_effort += file.root.local_effort;
					}
				}
			};
			file.each_node(compute_total_effort, Direction::Push);
		}

		// Push all tags for each file and to the includes. Keep pushing until no new tags are set into a file.root
		for (bool dirty = true; dirty; )
		{
			S("");
			dirty = false;
			for (auto &[_,file]: fp__file_)
			{
				auto push_tags = [&](auto &node, const auto &path){
					if (path.empty())
						return;
					const auto &parent = *path.back();
					L(node.path(path));
					for (const auto &[key,value]: parent.tags)
					{
						const auto p = node.tags.emplace(key, value);
						L(C(key)C(value)C(p.second));
						if (p.second)
						{
							for (const auto &incl_fp: node.includes)
							{
								auto it = fp__file_.find(incl_fp);
								if (it != fp__file_.end())
								{
									auto &file = it->second;
									const auto p = file.root.tags.emplace(key, value);
									if (p.second)
										dirty = true;
								}
							}
						}
					}
				};
				file.each_node(push_tags, Direction::Push);
			}
		}

		MSS_END();
	}

	void Library::print_debug(std::ostream &os) const
	{
		for (const auto &[fp,file]: fp__file_)
		{
			os << std::endl;
			os << "File " << fp << std::endl;
			auto print_node = [&](const auto &n, const auto &path){
				os << std::string(path.size(), ' ');
				for (const auto &meta: n.metas)
				{
					if (auto *state = std::get_if<meta::State>(&meta))
						os << "[State](" << *state << ")";
					else if (auto *effort = std::get_if<meta::Effort>(&meta))
						os << "[Effort](" << *effort << ")";
					else if (auto *duedate = std::get_if<meta::Duedate>(&meta))
						os << *duedate;
					else if (auto *urgency = std::get_if<meta::Urgency>(&meta))
						os << *urgency;
					else if (auto *command = std::get_if<meta::Command>(&meta))
						os << *command;
					else if (auto *tag = std::get_if<meta::Tag>(&meta))
						os << *tag;
				}
				os << " my:" << n.my_effort << " local:" << n.local_effort << " total:" << n.total_effort;
				for (const auto &[key,value]: n.tags)
					os << " " << key << ":" << value;
				os << "[Text](" << n.text << ")" << std::endl;
			};
			file.each_node(print_node, Direction::Push);
		}
	}

	bool Library::get(List &list, meta::Status wanted_status) const
	{
		MSS_BEGIN(bool);

		list.clear();

		for (const auto &p: fp__file_)
		{
			const auto &fp = p.first;
			const auto &file = p.second;

			auto append = [&](const auto &n, const auto &path){
				if (auto *state = n.template get<meta::State>(); !!state && state->status == wanted_status)
				{
					List::Item item;
					item.text = n.text;
					item.fp = fp;
					// item.state = *state;
					if (auto *ptr = n.template get<meta::Urgency>())
						item.urgency = ptr->value();
					if (auto *ptr = n.template get<meta::Effort>())
						item.effort = *ptr;
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
					if (auto *ptr = n.template get<meta::Urgency>())
						item.urgency = ptr->value();
					if (auto *ptr = n.template get<meta::Effort>())
						item.effort = *ptr;
					list.items.emplace_back(item);
				}
			};
			file.each_node(append, Direction::Push);
		}

		MSS_END();
	}

	// Privates
	bool Library::resolve_include_(std::filesystem::path &fp, std::string incl, const std::filesystem::path &context_fp) const
	{
		MSS_BEGIN(bool);

		using P = std::filesystem::path;

		MSS(config_.substitute_names(incl));

		P base = incl;
		if (!base.is_absolute())
			base = context_fp.parent_path() / incl;

		std::vector<P> candidates;
		if (!base.extension().empty())
		{
			candidates.push_back(base);
		}
		else
		{
			candidates.push_back(P{base}.replace_extension(".md"));
			candidates.push_back(P{base}.replace_extension(".naft"));
			candidates.push_back(P{base}.replace_extension(".mm"));
			candidates.push_back(P{base}.append("index").replace_extension(".md"));
			candidates.push_back(P{base}.append("index").replace_extension(".naft"));
			candidates.push_back(P{base}.append("index").replace_extension(".mm"));
		}

		std::optional<P> p;
		for (const auto &c: candidates)
		{
			if (!std::filesystem::is_regular_file(c))
				continue;
			MSS(!p, log::error() << "Ambiguous include '" << incl << "': both '" << *p << "' and '" << c << "' match" << std::endl);
			p = c;
		}
		MSS(!!p, log::error() << "Could not resolve include '" << incl << "'" << " from " << context_fp << std::endl);

		p->swap(fp);

		MSS_END();
	}

} 