#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/Node.hpp>
#include <dpn/enums.hpp>

#include <gubg/std/filesystem.hpp>

#include <string>
#include <vector>
#include <set>
#include <ostream>

namespace dpn { 

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		File() {}
		File(const std::string &text) {}

		std::filesystem::path fp;

		Node root;

		bool parse(const std::string &str, Format);

		bool interpret();

		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction) const
		{
			Path path;
			return each_node_(root, path, ftor, direction);
		}
		template <typename Ftor>
		void each_node(Ftor &&ftor, Direction direction)
		{
			Path path;
			return each_node_(root, path, ftor, direction);
		}

	private:
		template <typename Ftor>
		void each_node_(const Node &node, Path &path, Ftor &&ftor, Direction direction) const
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
		template <typename Ftor>
		void each_node_(Node &node, Path &path, Ftor &&ftor, Direction direction)
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

	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} 

#endif