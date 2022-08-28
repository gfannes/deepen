#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/Node.hpp>

#include <string>
#include <vector>
#include <ostream>

namespace dpn { 

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		Nodes nodes;

		bool parse(const std::string &str, Format);

		bool interpret();

		template <typename Ftor>
		void each_node(Ftor &&ftor) const
		{
			std::vector<const Node *> path;
			return each_node_(nodes, path, ftor);
		}
		template <typename Ftor>
		void each_node(Ftor &&ftor)
		{
			std::vector<Node *> path;
			return each_node_(nodes, path, ftor);
		}

	private:
		template <typename Ftor>
		void each_node_(const Nodes &nodes, std::vector<const Node *> &path, Ftor &&ftor) const
		{
			for (const auto &node: nodes)
			{
				ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					each_node_(node.childs, path, ftor);
					path.pop_back();
				}
			}
		}
		template <typename Ftor>
		void each_node_(Nodes &nodes, std::vector<Node *> &path, Ftor &&ftor)
		{
			for (auto &node: nodes)
			{
				ftor(node, path);
				if (!node.childs.empty())
				{
					path.push_back(&node);
					each_node_(node.childs, path, ftor);
					path.pop_back();
				}
			}
		}

	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} 

#endif