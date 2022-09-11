#include <dpn/Library.hpp>
#include <dpn/log.hpp>

#include <gubg/file/system.hpp>
#include <gubg/xml/Writer.hpp>
#include <gubg/mss.hpp>

#include <optional>
#include <set>
#include <fstream>

namespace dpn { 
	bool Library::Filter::operator()(const Node &node) const
	{
		MSS_BEGIN(bool);

		MSS_Q(!node.text.empty() || !node.childs.empty() || !node.all_dependencies.empty());

		MSS_Q(node.has_matching_tags(tags));

		if (status)
		{
			MSS_Q(!!node.agg_state);
			MSS_Q(node.agg_state->status == *status);
		}

		if (moscow)
		{
			MSS_Q(moscow->intersect(node.agg_moscow).any());
		}

		MSS_END();
	}

	void Library::clear()
	{
		fp__file_.clear();
	}

	bool Library::add_file(const std::filesystem::path &fp, bool is_root)
	{
		MSS_BEGIN(bool);

		if (is_root)
		{
			MSS(std::find_if(roots_.begin(), roots_.end(), [&](const auto &el){return el == fp;}) == roots_.end(), log::error() << "Root " << fp << " is already present" << std::endl);
			roots_.push_back(fp);
		}

		std::string content;
		MSS(gubg::file::read(content, fp), log::error() << "Could not read content from " << fp << std::endl);

		MSS(!fp__file_.count(fp));
		auto &file = fp__file_[fp];
		file.fp = fp;
		file.root.text = "[file]("+fp.string()+")";

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

		// Interpret each file and add all includes and requires recursively
		{
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
					file.each_node([&](auto &node, const auto &path){

						node__fp_[&node] = fp;

						for (const auto &meta: node.metas)
						{
							if (auto *command = std::get_if<meta::Command>(&meta))
							{
								if (const auto t = command->type; t == meta::Command::Include || t == meta::Command::Require)
								{
									const auto &dep = command->argument;
									std::filesystem::path dep_fp;
									if (!resolve_dependency_(dep_fp, dep, fp))
									{
										failures.push_back(dep);
										continue;
									}

									switch (t)
									{
										case meta::Command::Include: node.my_includes.insert(dep_fp); break;
										case meta::Command::Require: node.my_requires.insert(dep_fp); break;
										default: break;
									}
									node.all_dependencies.insert(dep_fp);

									if (!node.text.empty())
										log::warning() << "Found text in dependency node of " << fp << std::endl;
									if (node.my_urgency)
										log::warning() << "Found urgency in dependency node of " << fp << std::endl;

									if (fp__file_.count(dep_fp))
									{
										continue;
									}
									if (!add_file(dep_fp, false))
									{
										failures.push_back(dep);
										continue;
									}
								}
							}
						}

						// Merge the all_dependencies of all the childs into our own all_dependencies. This relies on Direction::Pull
						for (const auto &child: node.childs)
							node.all_dependencies.insert(child.all_dependencies.begin(), child.all_dependencies.end());
					}, Direction::Pull);

					MSS(failures.empty());

					done.insert(fp);
				}
			}
		}

		// Keep merging file.root.all_dependencies until things stabilize
		for (bool dirty = true; dirty; )
		{
			dirty = false;
			for (auto &[fp,file]: fp__file_)
			{
				const auto orig_size = file.root.all_dependencies.size();

				for (const auto &incl_fp: file.root.all_dependencies)
				{
					auto it = fp__file_.find(incl_fp);
					MSS(it != fp__file_.end());
					const auto &incl_file = it->second;
					file.root.all_dependencies.insert(incl_file.root.all_dependencies.begin(), incl_file.root.all_dependencies.end());
				}

				// Remove ourselves from root.all_dependencies
				file.root.all_dependencies.erase(fp);

				const auto new_size = file.root.all_dependencies.size();

				if (orig_size != new_size)
					dirty = true;
			}
		}

		// Update the Node.all_dependencies with the info for File.root
		for (auto &[_,file]: fp__file_)
		{
			auto complete_includes = [&](auto &node, const auto &path){
				const auto initial_includes = node.all_dependencies;
				for (const auto &initial_include: initial_includes)
				{
					auto it = fp__file_.find(initial_include);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						node.all_dependencies.insert(file.root.all_dependencies.begin(), file.root.all_dependencies.end());
						// Remove ourselves from all_dependencies
						if (auto fp_opt = get_fp(node); !!fp_opt)
							node.all_dependencies.erase(*fp_opt);
					}
				}
			};
			file.each_node(complete_includes, Direction::Pull);
		}

		// Push all tags for each file from root to leave and to the all_dependencies. Keep pushing until no new tags are set into a file.root
		for (bool dirty = true; dirty; )
		{
			dirty = false;
			for (auto &[_,file]: fp__file_)
			{
				auto push_tags = [&](auto &node, const auto &path){
					if (path.empty())
						return;
					const auto &parent = *path.back();
					for (const auto &[key,src_values]: parent.total_tags)
					{
						if (!node.my_tags.count(key))
						{
							auto &dst_values = node.total_tags[key];
							for (const auto &value: src_values)
							{
								const auto p = dst_values.emplace(value);
								if (p.second)
								{
									for (const auto &incl_fp: node.all_dependencies)
									{
										auto it = fp__file_.find(incl_fp);
										if (it != fp__file_.end())
										{
											auto &file = it->second;
											const auto p = file.root.total_tags[key].emplace(value);
											if (p.second)
												dirty = true;
										}
									}
								}
							}
						}
					}
				};
				file.each_node(push_tags, Direction::Push);
			}
		}

		// Aggregate the states and moscow from root to leaf
		{
			bool ok = true;
			auto aggregate = [&](auto &node, const auto &path){
				// Aggregate State
				if (node.my_state)
				{
					if (!node.agg_state)
						node.agg_state = node.my_state;
					AGG(ok, node.agg_state == node.my_state, log::error() << "State conflict detected with node.my_state" << std::endl);
				}
				else if (!path.empty())
				{
					const auto &parent = *path.back();
					if (parent.agg_state)
					{
						if (!node.agg_state)
							node.agg_state = parent.agg_state;
						AGG(ok, node.agg_state == parent.agg_state, log::error() << "State conflict detected with parent.agg_state" << std::endl);
					}
				}

				// Aggregate Moscow
				if (node.my_moscow)
				{
					node.agg_moscow = *node.my_moscow;
				}
				else if (!path.empty())
				{
					const auto &parent = *path.back();
					node.agg_moscow.merge(parent.agg_moscow);
				}
			};
			each_node(aggregate, Direction::Push);
			MSS(ok);
		}

		// Set my_effort to done if the state is done
		for (auto &[fp,file]: fp__file_)
		{
			auto set_my_effort_to_done = [&](auto &node, const auto &path){
				if (node.agg_state && node.agg_state->status == meta::Status::Done)
					node.my_effort.done = node.my_effort.total;
			};
			file.each_node(set_my_effort_to_done, Direction::Push);
		}

		compute_effort_([](Node &node) -> meta::Effort& {return node.total_effort;}, Filter{});

		MSS_END();
	}

	std::optional<std::filesystem::path> Library::get_fp(const Node &node) const
	{
		auto it = node__fp_.find(&node);
		if (it == node__fp_.end())
			return std::nullopt;
		return it->second;
	}

	void Library::print_debug(std::ostream &os) const
	{
		for (const auto &[fp,file]: fp__file_)
		{
			os << std::endl;
			os << "File " << fp << std::endl;
			auto print_node = [&](const auto &node, const auto &path){
				os << std::string(path.size(), ' ');
				for (const auto &meta: node.metas)
				{
					if (auto *state = std::get_if<meta::State>(&meta))
						os << "[State](" << *state << ")";
					else if (auto *moscow = std::get_if<meta::Moscow>(&meta))
						os << *moscow;
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
				os << " my:" << node.my_effort << " total:" << node.total_effort << " filtered:" << node.filtered_effort;
				for (const auto &[key,value]: node.my_tags)
					os << " " << key << ":" << value;
				if (node.my_state)
					os << " my: " << *node.my_state;
				if (node.agg_state)
					os << " agg: " << *node.agg_state;
				os << "[Text](" << node.text << ")" << std::endl;
			};
			file.each_node(print_node, Direction::Push);
		}
	}

	bool Library::get(List &list, const Filter &filter)
	{
		MSS_BEGIN(bool);

		list.clear();

		auto filter_wo_moscow = filter;
		filter_wo_moscow.moscow.reset();

		auto append = [&](const auto &node, const auto &path){
			if (!filter_wo_moscow(node))
				return;
			if (filter_wo_moscow.status)
			{
				if (node.my_state && node.my_state->status == *filter_wo_moscow.status)
				{
					auto &item = list.items.emplace_back(node);
					item.path = path;
				}
			}
		};
		each_node(append, Direction::Push);

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_due(List &list, const Filter &filter)
	{
		MSS_BEGIN(bool);

		list.clear();

		auto append = [&](const auto &node, const auto &path){
			if (!filter(node))
				return;
			if (auto *duedate = node.template get<meta::Duedate>())
			{
				auto &item = list.items.emplace_back(node);
				item.path = path;
			}
		};
		each_node(append, Direction::Push);

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_features(List &list, const Filter &filter)
	{
		MSS_BEGIN(bool);

		list.clear();

		auto lambda = [&](const auto &node, const auto &path){
			if (!filter(node))
				return;
			if (node.my_urgency)
			{
				auto &item = list.items.emplace_back(node);
				item.path = path;
			}
		};
		each_node(lambda, Direction::Push);

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_nodes_links(Id__Node &nodes, Id__DepIds &links) const
	{
		MSS_BEGIN(bool);

		nodes.clear();
		links.clear();

		std::map<const Node *, std::size_t> node__id;
		for (const auto &[_,file]: fp__file_)
		{
			auto add_node = [&](auto &node, const auto &_){
				if (auto p = node__id.emplace(&node, node__id.size()); p.second)
					nodes.push_back(&node);
			};
			file.each_node(add_node, Direction::Push);
		}

		const auto nr_nodes = node__id.size();
		for (const auto &[_,file]: fp__file_)
		{
			auto add_links = [&](auto &node, const auto &path){
				const auto me_id = node__id[&node];

				// Link from parent to me
				if (!path.empty())
				{
					const auto parent_id = node__id[path.back()];
					links[parent_id].insert(me_id);
				}

				// Link from child to previous child
				const Node *prev_child = nullptr;
				for (const auto &child: node.childs)
				{
					if (prev_child)
					{
						const auto child_id = node__id[&child];
						const auto prev_child_id = node__id[prev_child];
						links[child_id].insert(prev_child_id);
					}
					prev_child = &child;
				}

				// Link from me to all_dependencies
				for (const auto &incl_fp: node.my_includes)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						const auto dep_id = node__id[&file.root];
						links[me_id].insert(dep_id);
					}
				}
			};
			file.each_node(add_links, Direction::Push);
		}
		MSS(nr_nodes == node__id.size());

		MSS_END();
	}

	bool Library::export_mindmap(const std::string &root_text, const List &list, const Filter &filter, const std::filesystem::path &output_fp) const
	{
		MSS_BEGIN(bool);

		const auto folder = output_fp.parent_path();
		if (!folder.empty())
		{
			if (!std::filesystem::exists(folder))
				std::filesystem::create_directories(folder);
			MSS(std::filesystem::is_directory(folder), log::error() << "Could not create folder " << folder << std::endl);
		}

		std::ofstream fo{output_fp, std::ios::binary};

		gubg::xml::Writer writer{fo};
		auto map = writer.tag("map");
		map.attr("version", "freeplane 1.9.13");

		{
			auto root = map.tag("node");
			root.attr("TEXT", root_text);

			auto add_effort = [](const auto &effort, auto &tag){
				if (effort.total > 0)
				{
					tag.tag("attribute").attr("NAME", "todo").attr("VALUE", meta::Effort::to_dsl(effort.todo()));
					tag.tag("attribute").attr("NAME", "total").attr("VALUE", meta::Effort::to_dsl(effort.total));
					if (auto c = effort.completion())
						tag.tag("attribute").attr("NAME", "completion").attr("VALUE", std::to_string(*c)+"%");
				}
			};

			add_effort(list.effort, root);

			for (const auto &item: list.items)
			{
				std::list<gubg::xml::writer::Tag> tags;
				auto &feature = tags.emplace_back(root.tag("node"));
				feature.attr("TEXT", item.node().path(item.path));
				add_effort(item.node().filtered_effort, feature);

				Path prev_path;
				auto prev_path_in = [&](const auto &node, const auto &path){
					if (prev_path.empty())
						return true;
					auto ptr = prev_path.back();
					return ptr == &node || std::find(path.begin(), path.end(), ptr) != path.end();
				};

				auto lambda = [&](const auto &node, const auto &path){
					if (!filter(node))
						return;

					while (!prev_path_in(node, path))
					{
						prev_path.pop_back();
						tags.pop_back();
					}

					auto &n = tags.emplace_back(tags.back().tag("node"));
					n.attr("TEXT", node.text);
					add_effort(node.filtered_effort, n);

					prev_path = path;
					prev_path.push_back(&node);
				};
				for (const auto &child: item.node().childs)
					each_node(child, lambda, Direction::Push);

				// Close the xml tags in reverse order
				while (!tags.empty())
					tags.pop_back();
			}
		}

		MSS_END();
	}

	// Privates
	bool Library::resolve_dependency_(std::filesystem::path &fp, std::string incl, const std::filesystem::path &context_fp) const
	{
		MSS_BEGIN(bool);

		using P = std::filesystem::path;

		MSS(config_.substitute_names(incl));

		P base = incl;
		if (!base.is_absolute())
			base = context_fp.parent_path() / incl;
		base = base.lexically_normal();

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

	void Library::set_fps_(List &list)
	{
		for (auto &item: list.items)
		{
			if (auto fp_opt = get_fp(item.node()); !!fp_opt)
				item.fp = *fp_opt;
		}
	}

	void Library::compute_effort_(std::function<meta::Effort&(Node&)> get_effort, const Filter &filter)
	{
		// Compute the aggregate effort restricted to a single file, for all files and nodes
		for (auto &[fp,file]: fp__file_)
		{
			auto compute_local_effort = [&](auto &node, const auto &path){
				node.tmp_effort_.clear();
				if (filter(node))
					node.tmp_effort_ = node.my_effort;
				for (const auto &child: node.childs)
					node.tmp_effort_ += child.tmp_effort_;
			};
			// Direction::Pull is important to aggregate from leaf to root
			file.each_node(compute_local_effort, Direction::Pull);
		}

		// Compute the agg_effort as tmp_effort_ + tmp_effort_ of all all_dependencies
		// Note that agg_effort depends on the get_effort() getter
		// The Direction does not matter
		for (auto &[_,file]: fp__file_)
		{
			auto compute_total_effort = [&](auto &node, const auto &path){
				auto &agg_effort = get_effort(node);
				agg_effort = node.tmp_effort_;
				for (const auto &incl_fp: node.all_dependencies)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						agg_effort += file.root.tmp_effort_;
					}
				}
			};
			file.each_node(compute_total_effort, Direction::Push);
		}
	}

	void Library::compute_effort_(List &list, const Filter &filter)
	{
		compute_effort_([](Node &node) -> meta::Effort& {return node.filtered_effort;}, filter);

		list.effort.clear();

		std::set<const Node *> nodes;
		for (const auto &item: list.items)
		{
			const auto &node = item.node();

        	// Aggregate effort comprising the nodes in this list and all their descendants
			auto aggregate_effort = [&](const auto &node, const auto &_){
				if (!filter(node))
					return;
				if (nodes.count(&node) == 0)
				{
					list.effort += node.my_effort;
					nodes.insert(&node);
				}
			};
			each_node(node, aggregate_effort, Direction::Push);
		}
	}

} 