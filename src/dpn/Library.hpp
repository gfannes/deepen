#ifndef HEADER_dpn_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Library_hpp_ALREADY_INCLUDED

#include <dpn/config/Config.hpp>
#include <dpn/File.hpp>
#include <dpn/List.hpp>
#include <dpn/Options.hpp>
#include <dpn/enums.hpp>
#include <dpn/plan/types.hpp>
#include <dpn/plan/Graph.hpp>
#include <dpn/Tread.hpp>
#include <dpn/Filter.hpp>

#include <gubg/std/filesystem.hpp>
#include <gubg/map.hpp>

#include <map>
#include <ostream>
#include <functional>

namespace dpn { 

	class Library
	{
	public:
		Library(const config::Config &config, const Options &options): config_(config), options_(options) {}

		void clear();

		bool add_file(const std::filesystem::path &, bool is_root);

		bool resolve();

		std::optional<std::filesystem::path> get_fp(const Node &) const;

		bool get(List &, const Filter &);
		bool get_due(List &, const Filter &);
		bool get_features(List &, const Filter &);
		bool get_todo(List &, const Filter &);

        bool get_nodes_links(List &, Id__DepIds &, const Filter &) const;
        bool get_graph(List &nodes, Id__Id &part_of, Id__Id &after, Id__DepIds &requires, const Filter &) const;
        bool get_graph(plan::Graph &, const Filter &) const;

		bool export_mindmap(const std::string &root_text, const List &, const Filter &, const std::filesystem::path &) const;
		bool export_msproj(const List &, const Filter &, const std::filesystem::path &) const;
		bool export_msproj2(const plan::Graph &, const std::filesystem::path &) const;

		template <typename Ftor>
		void each_file(Ftor &&ftor){for (const auto &[_,file]: fp__file_) ftor(file);}

		// Can visit nodes multiple times
		template <typename Ftor>
		void each_node(const Node &root, Ftor &&ftor, Tread tread = {}) const
		{
			Path path;
			each_node_(root, path, ftor, tread);
		}
		template <typename Ftor>
		void each_node(Node &root, Ftor &&ftor, Tread tread = {})
		{
			Path path;
			each_node_(root, path, ftor, tread);
		}

		// Can visit nodes multiple times
		template <typename Ftor>
		void each_node(Ftor &&ftor, Tread tread = {}) const
		{
			for (const auto &root_fp: roots_)
			{
				if (auto file = gubg::get(fp__file_, root_fp))
				{
					if (tread.include_link_nodes)
					{
						each_node(file->root, ftor, tread);
					}
					else
					{
						// We do not iterate file->root since that is an artificial node
						for (const auto &child: file->root.childs)
							each_node(child, ftor, tread);
					}
				}
			}
		}
		template <typename Ftor>
		void each_node(Ftor &&ftor, Tread tread = {})
		{
			for (const auto &root_fp: roots_)
			{
				if (auto file = gubg::get(fp__file_, root_fp))
				{
					if (tread.include_link_nodes)
					{
						each_node(file->root, ftor, tread);
					}
					else
					{
						// We do not iterate file->root since that is an artificial node
						for (auto &child: file->root.childs)
							each_node(child, ftor, tread);
					}
				}
			}
		}


		template <typename Agg, typename... Args>
		void aggregate(std::list<Agg> &aggs, const Node &node, const Tread &tread, const Filter &filter, Args&... args) const
		{
			const auto do_process = filter(node) && (tread.include_link_nodes || node.type == Node::Type::Normal);
			Agg *parent_agg = nullptr;
			if (do_process)
			{
				if (!aggs.empty())
					parent_agg = &aggs.back();
				if (tread.direction == Direction::Push)
				{
					auto &dst = aggs.emplace_back(node, args...);
					if (parent_agg)
					{
						auto &src = *parent_agg;
						dst(src);
					}
				}
			}

			for (const auto &child: node.childs)
				aggregate<Agg>(aggs, child, tread, filter, args...);

			node.each_link(
				[&](const auto &link){
					if (auto file = gubg::get(fp__file_, link.fp))
						aggregate<Agg>(aggs, file->root, tread, filter.merge(link.filter), args...);
				},
				tread.dependency);

			if (do_process)
			{
				if (tread.direction == Direction::Pull)
				{
					auto &src = aggs.emplace_back(node, args...);
					if (parent_agg)
					{
						auto &dst = *parent_agg;
						dst(src);
					}
				}
				aggs.pop_back();
			}
		}
		template <typename Agg, typename... Args>
		void aggregate(std::list<Agg> &aggs, Node &node, const Tread &tread, const Filter &filter, Args&... args)
		{
			const auto do_process = filter(node) && (tread.include_link_nodes || node.type == Node::Type::Normal);
			Agg *parent_agg = nullptr;
			if (do_process)
			{
				if (!aggs.empty())
					parent_agg = &aggs.back();
				if (tread.direction == Direction::Push)
				{
					auto &dst = aggs.emplace_back(node, args...);
					if (parent_agg)
					{
						auto &src = *parent_agg;
						dst(src);
					}
				}
			}

			for (auto &child: node.childs)
				aggregate<Agg>(aggs, child, tread, filter, args...);

			node.each_link(
				[&](const auto &link){
					if (auto file = gubg::get(fp__file_, link.fp))
						aggregate<Agg>(aggs, file->root, tread, filter.merge(link.filter), args...);
				},
				tread.dependency);

			if (do_process)
			{
				if (tread.direction == Direction::Pull)
				{
					auto &src = aggs.emplace_back(node, args...);
					if (parent_agg)
					{
						auto &dst = *parent_agg;
						dst(src);
					}
				}
				aggs.pop_back();
			}
		}

		template <typename Agg, typename... Args>
		void aggregate(const Tread &tread, const Filter &filter, Args&... args) const
		{
			std::list<Agg> aggs;
			for (const auto &root_fp: roots_)
			{
				if (auto file = gubg::get(fp__file_, root_fp))
				{
					if (tread.include_link_nodes)
						aggregate<Agg>(aggs, file->root, tread, filter, args...);
					else
					{
						// We do not iterate file->root since that is an artificial node
						for (const auto &child: file->root.childs)
							aggregate<Agg>(aggs, child, tread, filter, args...);
					}
				}
			}
		}
		template <typename Agg, typename... Args>
		void aggregate(const Tread &tread, const Filter &filter, Args&... args)
		{
			std::list<Agg> aggs;
			for (const auto &root_fp: roots_)
			{
				if (auto file = gubg::get(fp__file_, root_fp))
				{
					if (tread.include_link_nodes)
						aggregate<Agg>(aggs, file->root, tread, filter, args...);
					else
					{
						// We do not iterate file->root since that is an artificial node
						for (auto &child: file->root.childs)
							aggregate<Agg>(aggs, child, tread, filter, args...);
					}
				}
			}
		}

		template <typename Ftor>
		void each_node2(Ftor &ftor, const Tread &tread, const Filter &filter) const
		{
			Path path;
			struct Lambda
			{
				const Node &node;
				Path &path;
				const Tread &tread;
				Ftor &ftor;
				Lambda(const Node &node, Path &path, const Tread &tread, Ftor &ftor): node(node), path(path), tread(tread), ftor(ftor)
				{
					if (tread.direction == Direction::Push)
						ftor(node, path);
					path.push_back(&node);
				}
				void operator()(const Lambda &) { }
				~Lambda()
				{
					path.pop_back();
					if (tread.direction == Direction::Pull)
						ftor(node, path);
				}
			};
			aggregate<Lambda>(tread, filter, path, tread, ftor);
		}

	private:
		template <typename Ftor>
		void each_node_(const Node &node, Path &path, Ftor &&ftor, const Tread &tread) const
		{
			{
				const auto it = std::find_if(path.begin(), path.end(), [&](auto ptr){return ptr == &node;});
				if (it != path.end())
					// This is a circular dependency
					return;
			}

			unsigned int count = 0;
			auto lambda = [&](const auto &link){
				if (count == 0)
				{
					if (tread.include_link_nodes && tread.direction == Direction::Push)
						ftor(node, path);
				}

				if (auto file = gubg::get(fp__file_, link.fp))
				{
					if (tread.include_link_nodes)
					{
						path.push_back(&node);
						each_node_(file->root, path, ftor, tread);
						path.pop_back();
					}
					else if (!file->root.childs.empty())
					{
						// We do not iterate file->root since that is an artificial node
						for (const auto &child: file->root.childs)
							each_node_(child, path, ftor, tread);
					}
				}

				++count;
			};
			node.each_link(lambda, tread.dependency);

			if (count > 0)
			{
				if (tread.include_link_nodes && tread.direction == Direction::Pull)
					ftor(node, path);
			}
			else
			{
				if (tread.direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					for (const auto &child: node.childs)
						each_node_(child, path, ftor, tread);
					path.pop_back();
				}
				if (tread.direction == Direction::Pull)
					ftor(node, path);
			}
		}
		template <typename Ftor>
		void each_node_(Node &node, Path &path, Ftor &&ftor, const Tread &tread)
		{
			{
				const auto it = std::find_if(path.begin(), path.end(), [&](auto ptr){return ptr == &node;});
				if (it != path.end())
					// This is a circular dependency
					return;
			}

			unsigned int count = 0;
			auto lambda = [&](const auto &link){
				if (count == 0)
				{
					if (tread.include_link_nodes && tread.direction == Direction::Push)
						ftor(node, path);
				}

				if (auto file = gubg::get(fp__file_, link.fp))
				{
					if (tread.include_link_nodes)
					{
						path.push_back(&node);
						each_node_(file->root, path, ftor, tread);
						path.pop_back();
					}
					else if (!file->root.childs.empty())
					{
						// We do not iterate file->root since that is an artificial node
						for (auto &child: file->root.childs)
							each_node_(child, path, ftor, tread);
					}
				}

				++count;
			};
			node.each_link(lambda, tread.dependency);

			if (count > 0)
			{
				if (tread.include_link_nodes && tread.direction == Direction::Pull)
					ftor(node, path);
			}
			else
			{
				if (tread.direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					for (auto &child: node.childs)
						each_node_(child, path, ftor, tread);
					path.pop_back();
				}
				if (tread.direction == Direction::Pull)
					ftor(node, path);
			}
		}

		bool resolve_dependencies_(std::vector<std::filesystem::path> &fps, std::string incl, const std::filesystem::path &context_fp) const;

		void set_fps_(List &list);
		void compute_effort_(std::function<meta::Effort&(Node&)> get_effort, const Filter &);
		void compute_effort_(List &list, const Filter &);

		const config::Config &config_;
		const Options &options_;

		std::vector<std::filesystem::path> roots_;
		std::map<std::filesystem::path, File> fp__file_;
		std::map<const Node *, std::filesystem::path> node__fp_;
	};

} 

#endif