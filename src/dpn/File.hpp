#ifndef HEADER_dpn_File_hpp_ALREADY_INCLUDED
#define HEADER_dpn_File_hpp_ALREADY_INCLUDED

#include <dpn/onto/Node.hpp>

#include <string>
#include <ostream>

namespace dpn { 

	class File
	{
	public:
		enum class Format {Markdown, Naft, Freemind};

		onto::Nodes nodes;

		bool parse(const std::string &str, Format);

		bool interpret();

		template <typename Ftor>
		void each_node(Ftor &&ftor) { return each_node_(nodes, ftor); }

	private:
		template <typename Ftor>
		void each_node_(onto::Nodes &nodes, Ftor &&ftor)
		{
			for (auto &node: nodes)
			{
				ftor(node);
				each_node_(node.childs, ftor);
			}
		}

	};

	std::ostream &operator<<(std::ostream &os, File::Format format);

} 

#endif