#ifndef HEADER_dpn_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Library_hpp_ALREADY_INCLUDED

#include <dpn/config/Config.hpp>
#include <dpn/File.hpp>
#include <dpn/List.hpp>
#include <dpn/Options.hpp>
#include <dpn/enums.hpp>

#include <gubg/std/filesystem.hpp>

#include <map>
#include <set>
#include <ostream>
#include <functional>

namespace dpn { 

	using Id__DepIds = std::map<std::size_t, std::set<std::size_t>>;

	class Library
	{
	public:
		struct Filter
		{
			Tags tags;
			std::optional<meta::Status> status;
			std::optional<meta::Moscow> moscow;

			bool operator()(const Node &) const;
		};

		Library(const config::Config &config, const Options &options): config_(config), options_(options) {}

		void clear();

		bool add_file(const std::filesystem::path &, bool is_root);

		bool resolve();

		std::optional<std::filesystem::path> get_fp(const Node &) const;

		void print_debug(std::ostream &) const;

		bool get(List &, const Filter &);
		bool get_due(List &, const Filter &);
		bool get_features(List &, const Filter &);

        bool get_nodes_links(List &, Id__DepIds &, const Filter &) const;

		bool export_mindmap(const std::string &root_text, const List &list, const Filter &filter, const std::filesystem::path &output_fp) const;

		template <typename Ftor>
		void each_file(Ftor &&ftor){for (const auto &[_,file]: fp__file_) ftor(file);}

		// Can visit nodes multiple times
		template <typename Ftor>
		void each_node(const Node &root, Ftor &&ftor, Direction direction) const
		{
			Path path;
			each_node_(root, path, ftor, direction);
		}
		template <typename Ftor>
		void each_node(Node &root, Ftor &&ftor, Direction direction)
		{
			Path path;
			each_node_(root, path, ftor, direction);
		}

		// Can visit nodes multiple times
		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction) const
		{
			for (const auto &root_fp: roots_)
			{
				const auto it = fp__file_.find(root_fp);
				if (it != fp__file_.end())
				{
					const auto &file = it->second;
					// We do not iterate file.root since that is an artificial node
					for (const auto &child: file.root.childs)
						each_node(child, ftor, direction);
				}
			}
		}
		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction)
		{
			for (const auto &root_fp: roots_)
			{
				const auto it = fp__file_.find(root_fp);
				if (it != fp__file_.end())
				{
					auto &file = it->second;
					// We do not iterate file.root since that is an artificial node
					for (auto &child: file.root.childs)
						each_node(child, ftor, direction);
				}
			}
		}

	private:
		template <typename Ftor>
		void each_node_(const Node &node, Path &path, Ftor &&ftor, Direction direction) const
		{
			{
				const auto it = std::find_if(path.begin(), path.end(), [&](auto ptr){return ptr == &node;});
				if (it != path.end())
					// This is a circular dependency
					return;
			}

			if (!node.my_includes.empty())
			{
				for (const auto &dep_fp: node.my_includes)
				{
					const auto it = fp__file_.find(dep_fp);
					if (it != fp__file_.end())
					{
						const auto &file = it->second;
						// We do not iterate file.root since that is an artificial node
						for (const auto &child: file.root.childs)
							each_node_(child, path, ftor, direction);
					}
				}
			}
			else
			{
				if (direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					for (const auto &child: node.childs)
						each_node_(child, path, ftor, direction);
					path.pop_back();
				}
				if (direction == Direction::Pull)
					ftor(node, path);
			}
		}
		template <typename Ftor>
		void each_node_(Node &node, Path &path, Ftor &&ftor, Direction direction)
		{
			{
				const auto it = std::find_if(path.begin(), path.end(), [&](auto ptr){return ptr == &node;});
				if (it != path.end())
					// This is a circular dependency
					return;
			}

			if (!node.my_includes.empty())
			{
				for (const auto &dep_fp: node.my_includes)
				{
					const auto it = fp__file_.find(dep_fp);
					if (it != fp__file_.end())
					{
						auto &file = it->second;
						// We do not iterate file.root since that is an artificial node
						for (auto &child: file.root.childs)
							each_node_(child, path, ftor, direction);
					}
				}
			}
			else
			{
				if (direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					for (auto &child: node.childs)
						each_node_(child, path, ftor, direction);
					path.pop_back();
				}
				if (direction == Direction::Pull)
					ftor(node, path);
			}
		}

		bool resolve_dependency_(std::filesystem::path &fp, std::string incl, const std::filesystem::path &context_fp) const;

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