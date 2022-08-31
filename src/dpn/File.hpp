#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/Node.hpp>

#include <gubg/std/filesystem.hpp>

#include <string>
#include <vector>
#include <set>
#include <ostream>

namespace dpn { 

	enum class Direction {Push, Pull};

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		Nodes nodes;

		std::set<std::filesystem::path> includes;

		meta::Effort local_effort;
		meta::Effort total_effort;

		bool parse(const std::string &str, Format);

		bool interpret();

		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction) const
		{
			std::vector<const Node *> path;
			return each_node_(nodes, path, ftor, direction);
		}
		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction)
		{
			std::vector<Node *> path;
			return each_node_(nodes, path, ftor, direction);
		}

	private:
		template <typename Ftor>
		void each_node_(const Nodes &nodes, std::vector<const Node *> &path, Ftor &&ftor, Direction direction) const
		{
			for (const auto &node: nodes)
			{
				if (direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					each_node_(node.childs, path, ftor, direction);
					path.pop_back();
				}
				if (direction == Direction::Pull)
					ftor(node, path);
			}
		}
		template <typename Ftor>
		void each_node_(Nodes &nodes, std::vector<Node *> &path, Ftor &&ftor, Direction direction)
		{
			for (auto &node: nodes)
			{
				if (direction == Direction::Push)
					ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					each_node_(node.childs, path, ftor, direction);
					path.pop_back();
				}
				if (direction == Direction::Pull)
					ftor(node, path);
			}
		}

	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} 

#endif