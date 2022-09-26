#include <dpn/Library.hpp>
#include <dpn/log.hpp>

#include <gubg/file/system.hpp>
#include <gubg/xml/Writer.hpp>
#include <gubg/xml/Escaper.hpp>
#include <gubg/map.hpp>
#include <gubg/hr.hpp>
#include <gubg/mss.hpp>

#include <optional>
#include <set>
#include <fstream>

namespace dpn { 

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

			std::map<std::filesystem::path, std::filesystem::path> fp__includer;
			for (std::vector<std::filesystem::path> fps_todo; get_todos(fps_todo); )
			{
				for (const auto &fp: fps_todo)
				{
					auto &file = fp__file_[fp];
					file.root.type = Node::Type::Root;

					MSS(file.interpret(), log::error() << "Could not interpret " << fp << std::endl);

					std::vector<std::string> failures;
					file.each_node([&](auto &node, const auto &_){

						node__fp_[&node] = fp;

						for (const auto &meta: node.metas)
						{
							if (auto *command = std::get_if<meta::Command>(&meta))
							{
								if (const auto t = command->type; t == meta::Command::Include || t == meta::Command::Require)
								{
									switch (t)
									{
										case meta::Command::Include:
										node.type = Node::Type::Link;
										if (!node.text.empty())
											log::warning() << "Found text in dependency node of " << fp << std::endl;
										node.text = "[include]";
										break;

										case meta::Command::Require:
										node.type = Node::Type::Link;
										if (!node.text.empty())
											log::warning() << "Found text in dependency node of " << fp << std::endl;
										node.text = "[require]";
										break;

										default: break;
									}

									// Split the arguments in Tags and dependencies that should be resolved
									// The Tags will filter the imported data
									std::list<std::string> deps;
									TagSets all_tags;
									for (const auto &dep: command->arguments)
									{
										meta::Tag::Opt tag;
										gubg::Strange strange{dep};
										if (!parse(tag, strange))
											failures.push_back(dep);
										if (tag)
											all_tags[tag->key].insert(tag->value);
										else
											deps.push_back(dep);
									}

									for (const auto &dep: deps)
									{
										std::vector<std::filesystem::path> dep_fps;
										if (!resolve_dependencies_(dep_fps, dep, fp))
										{
											failures.push_back(dep);
											continue;
										}

										for (const auto &dep_fp: dep_fps)
										{
											switch (t)
											{
												case meta::Command::Include:
												{
													auto p = fp__includer.emplace(dep_fp, fp);
													if (!p.second)
													{
														// An `include` relationship is the same as a parent-child relationship. Hence, we enforce only a single include per fp.
														failures.push_back(dep);
														log::error() << "File " << dep_fp << " is already included from " << p.first->second << ", cannot include it again from " << fp << std::endl;
													}
													node.my_includes.insert(dep_fp);
													auto &link = goc(node.links, Link::Include, dep_fp);
													link.filter.all_tags = all_tags;
												}
												break;

												case meta::Command::Require:
												{
													node.my_requires.insert(dep_fp);
													auto &link = goc(node.links, Link::Require, dep_fp);
													link.filter.all_tags = all_tags;
												}
												break;

												default: break;
											}
											node.text += "("+dep_fp.string()+")";
											node.all_dependencies.insert(dep_fp);

											if (node.my_urgency)
												log::warning() << "Found urgency in dependency node of " << fp << std::endl;
											if (!node.childs.empty())
												log::warning() << "Found childs in dependency node of " << fp << std::endl;

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
			auto complete_includes = [&](auto &node, const auto &_){
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

		// Setup node.parent information based on within-file path and node.my_includes information
		for (auto &[_,file]: fp__file_)
		{
			bool ok = true;
			auto setup = [&](auto &node, auto &path){
				if (!path.empty())
					node.parent = path.back();

				for (const auto &fp: node.my_includes)
					gubg::with_value(fp__file_, fp, [&](auto &file){
						AGG(ok, !file.root.parent, log::error() << "File " << fp << " already has a parent: " << *file.root.parent << std::endl);
						file.root.parent = &node;
					});
			};
			file.each_node(setup, Direction::Push);
			MSS(ok);
		}

		// Push all tags for each file from root to leave and to the dependencies. Keep pushing until no new tags are set into a file.root
		{
			auto setup_all_tags = [](auto &node, const auto &_){
				for (const auto &[key,value]: node.my_tags)
					node.all_tags[key].insert(value);
			};
			each_node(setup_all_tags, Tread{.dependency = Dependency::Mine});
		}
		for (bool dirty = true; dirty; )
		{
			dirty = false;
			for (auto &[_,file]: fp__file_)
			{
				auto push_tags = [&](auto &node, const auto &path){
					if (path.empty())
						return;
					const auto &parent = *path.back();

					for (auto p: parent.all_tags)
					{
						const auto &key = p.first;
						const auto &src_values = p.second;

						if (node.my_tags.count(key))
							// If the key occurs in my_tags, we do not merge the parent values for this key
							continue;

						// Merge the parent tags into node
						auto &dst_values = node.all_tags[key];
						for (const auto &value: src_values)
						{
							const auto p = dst_values.emplace(value);
							if (p.second)
							{
								auto push_to_dependency = [&](const auto &link){
									if (auto file = gubg::get(fp__file_, link.fp))
									{
										const auto p = file->root.all_tags[key].emplace(value);
										if (p.second)
											dirty = true;
									}
								};
								node.each_link(push_to_dependency, Dependency::Mine);
							}
						}
					}
				};
				file.each_node(push_tags, Direction::Push);
			}
		}

		// Aggregate the states, moscow and urgency from root to leaf
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

				// Aggregate Urgency
				if (node.my_urgency)
				{
					node.agg_urgency = *node.my_urgency;
				}
				else if (!path.empty())
				{
					const auto &parent = *path.back();
					node.agg_urgency.merge(parent.agg_urgency);
				}
			};
			each_node(aggregate, Tread{.dependency = Dependency::Mine});
			MSS(ok);
		}

		// Set my_effort to done if the state is done
		for (auto &[fp,file]: fp__file_)
		{
			auto set_my_effort_to_done = [&](auto &node, const auto &_){
				if (node.agg_state && node.agg_state->status == meta::Status::Done)
					node.my_effort.done = node.my_effort.total;
			};
			file.each_node(set_my_effort_to_done, Direction::Push);
		}

		compute_effort_([](Node &node) -> meta::Effort& {return node.total_effort;}, Filter{});

		// Push sequence from root to leaf, but only within a file, not to all dependencies
		for (auto &[_,file]: fp__file_)
		{
			auto aggregate_sequence = [](auto &node, const auto &path){
				if (node.my_sequence)
					node.agg_sequence = *node.my_sequence;
				else if (!path.empty())
					node.agg_sequence = path.back()->agg_sequence;
			};
			file.each_node(aggregate_sequence, Direction::Push);
		}

		MSS_END();
	}

	std::optional<std::filesystem::path> Library::get_fp(const Node &node) const
	{
		auto it = node__fp_.find(&node);
		if (it == node__fp_.end())
			return std::nullopt;
		return it->second;
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
		each_node(append, Tread{.dependency = Dependency::Mine});

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
		each_node(append, Tread{.dependency = Dependency::Mine});

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_features(List &list, const Filter &filter)
	{
		MSS_BEGIN(bool);

		list.clear();

		std::set<const Node *> nodes;
		auto lambda = [&](const auto &node, const auto &path){
			auto p = nodes.insert(&node);
			if (!p.second)
				return;

			if (!filter(node))
				return;

			if (node.my_urgency && node.my_urgency->is_public)
			{
				auto &item = list.items.emplace_back(node);
				item.path = path;
			}

			auto detect_excluded_dependencies = [&](const auto &link){
				if (auto file = gubg::get(fp__file_, link.fp))
				{
					const auto &root = file->root;
					if (!filter(root))
						std::cout << "Found dropped dependency from " << node.path(path) << " on " << root.text << std::endl;
				}
			};
			node.each_link(detect_excluded_dependencies, Dependency::Mine);
		};
		each_node(lambda, Tread{.dependency = Dependency::Mine});

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_todo(List &list, const Filter &filter)
	{
		MSS_BEGIN(bool);

		list.clear();

		auto lambda = [&](const auto &node, const auto &path){
			if (!filter(node))
				return;
			if (node.is_heading && node.total_effort.todo() > 0)
			{
				auto &item = list.items.emplace_back(node);
				item.path = path;
			}
		};
		each_node(lambda, Tread{.dependency = Dependency::Mine});

		set_fps_(list);
		compute_effort_(list, filter);

		MSS_END();
	}

	bool Library::get_nodes_links(List &nodes, Id__DepIds &links, const Filter &filter) const
	{
		MSS_BEGIN(bool);

		nodes.clear();
		links.clear();

		std::map<const Node *, std::size_t> node__id;
		auto add_node = [&](auto &node, const auto &path){
			if (!filter(node))
				return;

			if (auto p = node__id.emplace(&node, node__id.size()); p.second)
			{
				auto &item = nodes.items.emplace_back(node);
				item.path = path;
			}
		};
		each_node(add_node, Tread{.dependency = Dependency::Mine, .include_link_nodes = true});

		const auto nr_nodes = node__id.size();
		for (const auto &[_,file]: fp__file_)
		{
			auto add_links = [&](auto &node, const auto &path){
				if (!filter(node))
					return;

				const auto me_id = node__id[&node];

				// Link from parent to me
				if (!path.empty())
				{
					const auto &parent = *path.back();
					if (filter(parent))
					{
						const auto parent_id = node__id[&parent];
						links[parent_id].insert(me_id);
					}
				}

				// Link from child to previous child
				if (!node.agg_sequence.any)
				{
					std::optional<std::size_t> prev_child_id;
					for (const auto &child: node.childs)
					{
						if (filter(child))
						{
							const auto child_id = node__id[&child];
							if (prev_child_id)
								links[child_id].insert(*prev_child_id);
							prev_child_id = child_id;
						}
					}
				}

				// Link from me to my_includes, and from my_includes to my links
				for (const auto &incl_fp: node.my_includes)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						if (filter(file.root))
						{
							const auto dep_id = node__id[&file.root];
							links[me_id].insert(dep_id);
						}
					}
				}
			};
			file.each_node(add_links, Direction::Push);
		}
		MSS(nr_nodes == node__id.size(), log::internal_error() << "More nodes were added" << std::endl);

		MSS_END();
	}

	bool Library::get_graph(List &nodes, Id__Id &part_of, Id__Id &after, Id__DepIds &requires, const Filter &filter) const
	{
		MSS_BEGIN(bool);

		nodes.clear();
		part_of.clear();
		after.clear();
		requires.clear();

		std::map<const Node *, std::size_t> node__id;
		auto add_node = [&](auto &node, const auto &path){
			if (!filter(node))
				return;

			if (auto p = node__id.emplace(&node, node__id.size()); p.second)
			{
				auto &item = nodes.items.emplace_back(node);
				item.path = path;
			}
		};
		each_node(add_node, Tread{.dependency = Dependency::Include, .include_link_nodes = true});
		each_node(add_node, Tread{.dependency = Dependency::Mine, .include_link_nodes = true});

		const auto nr_nodes = node__id.size();
		for (const auto &[_,file]: fp__file_)
		{
			auto add_links = [&](auto &node, const auto &path){
				if (!filter(node))
					return;

				const auto my_id = node__id[&node];

				// node is part of its parent, if any
				if (!path.empty())
				{
					const auto &parent = *path.back();
					if (filter(parent))
					{
						const auto parent_id = node__id[&parent];
						part_of[my_id] = parent_id;
					}
				}

				// sibling dependencies
				if (!node.agg_sequence.any)
				{
					std::optional<std::size_t> prev_child_id;
					for (const auto &child: node.childs)
					{
						if (filter(child))
						{
							const auto child_id = node__id[&child];
							if (prev_child_id)
							{
								// child comes after prev_child
								after[child_id] = *prev_child_id;
							}
							prev_child_id = child_id;
						}
					}
				}

				// my_includes are part of node
				for (const auto &incl_fp: node.my_includes)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						if (filter(file.root))
						{
							const auto dep_id = node__id[&file.root];
							part_of[dep_id] = my_id;
						}
					}
				}

				// my_requires are only used by node, but not part.
				// Eg, they can start before node starts
				for (const auto &incl_fp: node.my_requires)
				{
					auto it = fp__file_.find(incl_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						if (filter(file.root))
						{
							const auto dep_id = node__id[&file.root];
							requires[my_id].insert(dep_id);
						}
					}
				}
			};
			file.each_node(add_links, Direction::Push);
		}
		MSS(nr_nodes == node__id.size(), log::internal_error() << "More nodes were added" << std::endl);

		MSS_END();
	}

	bool Library::get_graph(plan::Graph &graph, const Filter &filter) const
	{
		MSS_BEGIN(bool);

		graph.clear();

		Id next_id_ = 0;
		auto generate_id = [&](){
			const auto id = next_id_;
			// We leave room for a potential self-node: an artificial leaf node that
			// contains the effort of a non-leaf node
			next_id_ += 2;
			return id;
		};

		std::set<const Node *> known_nodes;
		// We keep a path based on Ids ourselves
		std::vector<Id> id_path;
		auto assign_id = [&](const auto &node, const auto &path){
			// We create a Vertex for every Node we observe, even if we observe the same Node multiple times
			// To ensure the effort aggregation is correct, we assume that the first user of a feature pays
			const auto my_id = generate_id();

			graph.vertices.insert(my_id);
			graph.text[my_id] = node.text;
			graph.depth[my_id] = path.size();
			if (known_nodes.count(&node) == 0)
			{
				// First user pays
				graph.my_effort[my_id] = node.my_effort.todo();
				graph.agg_effort[my_id] = graph.my_effort[my_id];
			}

			// Setup graph.sibling and correct the id_path before it is used hereunder
			if (id_path.size() > path.size())
			{
				if (path.empty() || !path.back()->agg_sequence.any)
					graph.fs[my_id].insert(id_path[path.size()]);
			}
			id_path.resize(path.size());

			// Setup graph.parent
			if (!id_path.empty())
				graph.parent[my_id] = id_path.back();

			known_nodes.insert(&node);
			id_path.push_back(my_id);
		};

		const Tread tread = {.dependency = Dependency::Mine, .include_link_nodes = false};
		each_node2(assign_id, tread, filter);

		// Aggregate the effort
		for (auto it = graph.vertices.rbegin(); it != graph.vertices.rend(); ++it)
		{
			const auto my_id = *it;
			if (auto parent = gubg::get(graph.parent, my_id))
				graph.agg_effort[*parent] += graph.agg_effort[my_id];
		}

		// Prune leaf Vertices without effort
		std::set<Id> non_leafs;
		for (bool again = true; again; )
		{
			// Identify the Vertices that are someone's parent: these are not leafs
			non_leafs.clear();
			for (auto id: graph.vertices)
			{
				if (auto parent = gubg::get(graph.parent, id))
					non_leafs.insert(*parent);
			}

			std::set<Id> empty_leafs;
			for (auto id: graph.vertices)
			{
				if (non_leafs.count(id))
					continue;
				auto effort = gubg::get(graph.my_effort, id);
				if (!effort || *effort == 0)
					empty_leafs.insert(id);
			}

			for (auto id: empty_leafs)
				graph.erase(id);

			again = !empty_leafs.empty();
		}

		// Create leaf nodes for non-leafs with my_effort != 0
		for (auto id: non_leafs)
		{
			auto effort = gubg::get(graph.my_effort, id);
			if (!effort || *effort == 0)
				continue;

			const auto my_id = id+1;

			graph.vertices.insert(my_id);
			graph.text[my_id] = graph.text[id] + " [self]";
			graph.depth[my_id] = graph.depth[id]+1;
			graph.my_effort[my_id] = graph.my_effort[id];
			graph.agg_effort[my_id] = graph.my_effort[my_id];
			graph.my_effort[id] = 0;
			graph.parent[my_id] = id;
		}

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

			std::set<const Node *> already_included;

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
					if (already_included.count(&node))
						return;
					already_included.insert(&node);

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
					each_node(child, lambda, Tread{.dependency = Dependency::Mine});

				// Close the xml tags in reverse order
				while (!tags.empty())
					tags.pop_back();
			}
		}

		MSS_END();
	}

	bool Library::export_msproj(const List &list, const Filter &filter, const std::filesystem::path &output_fp) const
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
		writer.prolog("version", "1.0", "encoding", "UTF-8", "standalone", "yes");
		auto project = writer.tag("Project");
		project.attr("xmlns", "http://schemas.microsoft.com/project");
		auto tasks = project.tag("Tasks");

		auto add_task = [&](unsigned int &uid, const auto &text, unsigned int depth, unsigned int minutes){
			auto task = tasks.tag("Task");
			task.tag("UID") << uid;
			task.tag("Name") << (!text.empty() ? text : "[empty]");
			task.tag("OutlineLevel") << depth;
			if (minutes > 0)
			{
				task.tag("DurationFormat") << 53;
				{
					const std::string pt = std::string("PT0H")+std::to_string(minutes)+"M0S";
					task.tag("Duration") << pt;
					task.tag("Work") << pt;
				}
			}

			++uid;
		};

		std::set<const Node *> already_added;

		unsigned int uid = 0;
		for (auto ix = 0u; ix < list.items.size(); ++ix)
		{
			const auto &item = list.items[ix];

			auto lambda = [&](const auto &node, const auto &path){
				if (!already_added.insert(&node).second)
					return;
				if (!filter(node))
					return;

				const auto effort = node.filtered_effort;
				if (effort.todo() == 0)
					return;
				if (node.childs.empty())
				{
					// Leaf node
					add_task(uid, node.text, path.size()+2, node.my_effort.todo()*15);
				}
				else
				{
					// Not a leaf node
					if (node.my_effort.todo() > 0 && node.filtered_effort.todo() > node.my_effort.todo())
					{
						add_task(uid, node.text, path.size()+2, 0);
						add_task(uid, node.text+" (self)", path.size()+3, node.my_effort.todo()*15);
					}
					else
						add_task(uid, node.text, path.size()+2, node.my_effort.todo()*15);
				}
			};
			each_node(item.node(), lambda, Tread{.dependency = Dependency::Mine});
		}

		MSS_END();
	}

	// Warning: still wip
	// .? Start from get_features
	// .? Prune branches without effort
	bool Library::export_msproj2(const plan::Graph &graph, const std::filesystem::path &output_fp) const
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
		writer.prolog("version", "1.0", "encoding", "UTF-8", "standalone", "yes");
		auto project = writer.tag("Project");
		project.attr("xmlns", "http://schemas.microsoft.com/project");
		auto tasks = project.tag("Tasks");

		gubg::xml::Escaper escaper;

		for (auto id: graph.vertices)
		{
			auto task = tasks.tag("Task");
			task.tag("UID") << id;

			const auto &text_esc = escaper.escape(*gubg::get(graph.text, id));
			task.tag("Name") << (!text_esc.empty() ? text_esc : "[empty]");

			// Merlin Project does not accept nodes with OutlineLevel == 0, hence the +1
			task.tag("OutlineLevel") << *gubg::get(graph.depth, id)+1;

			if (auto minutes_ptr = gubg::get(graph.my_effort, id))
			{
				const auto minutes = *minutes_ptr*15;
				task.tag("DurationFormat") << 53;
				{
					const std::string pt = std::string("PT0H")+std::to_string(minutes)+"M0S";
					task.tag("Duration") << pt;
					task.tag("Work") << pt;
				}
				if (minutes == 0)
					task.tag("Milestone") << 1;
			}
			gubg::with_value(graph.fs, id, [&](const auto &oids){
				for (auto oid: oids)
				{
					auto predecessor_link = task.tag("PredecessorLink");
					predecessor_link.tag("PredecessorUID") << oid;
					predecessor_link.tag("Type") << 1;
				}
			});
		}

		MSS_END();
	}

	// Privates
	bool Library::resolve_dependencies_(std::vector<std::filesystem::path> &fps, std::string incl, const std::filesystem::path &context_fp) const
	{
		MSS_BEGIN(bool);

		using P = std::filesystem::path;

		MSS(config_.substitute_names(incl));

		P base = incl;
		if (!base.is_absolute())
			base = context_fp.parent_path() / incl;
		base = base.lexically_normal();

		unsigned int push_count = 0;
		auto push_fp = [&](const auto &fp){
			if (std::filesystem::is_regular_file(fp))
			{
				if (fp != context_fp)
				{
					fps.push_back(fp);
					++push_count;
				}
			}
			return true;
		};

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

		for (const auto &candidate: candidates)
		{
			// Transfer parts from base to dir and rel:
			// * dir receives everything before the globbing symbol '*' is encountered
			// * rel receives the rest
			std::filesystem::path dir;
			std::filesystem::path rel;
			auto dst = &dir;
			for (const auto part: candidate)
			{
				if (part.string().find('*') != std::string::npos)
					dst = &rel;
				*dst /= part;
			}

			if (dst == &dir)
				// No globbing found
				push_fp(dir);
			else
				// Globbing was found
				gubg::file::each_glob(rel.string(), push_fp, dir);
		}

		MSS(push_count > 0, log::error() << "Could not resolve '" << incl << "' from " << context_fp << std::endl);

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
			each_node(node, aggregate_effort, Tread{.dependency = Dependency::Mine});
		}
	}

} 