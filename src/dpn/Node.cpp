#include <dpn/Node.hpp>

#include <gubg/naft/Document.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/hr.hpp>

namespace dpn { 

	unsigned int Node::depth(bool include_roots) const
	{
		if (!parent)
			return 0;

		if (!include_roots && type == Type::Root)
			return parent->depth(include_roots);

		return parent->depth(include_roots)+1;
	}

	std::string Node::path(const Path &path, char sep) const
	{
		std::string str;
		gubg::OnlyOnce skip_sep;
		for (const auto ptr: path)
		{
			if (!skip_sep())
				str.push_back(sep);
			str += ptr->text;
		}
		if (!skip_sep())
			str.push_back(sep);
		str += text;
		return str;
	}

	std::string to_string(const Path &path, char sep)
	{
		std::string str;
		gubg::OnlyOnce skip_sep;
		for (const auto ptr: path)
		{
			if (!skip_sep())
				str.push_back(sep);
			str += ptr->text;
		}
		return str;
	}

	std::ostream &operator<<(std::ostream &os, const Node &node)
	{
		gubg::naft::Document doc{os};
		auto n = doc.node("Node");
		n.attr("text", node.text);
		n.attr("depth", node.depth(true));
		n.attr("parent", node.parent);
		n.attr("all_tags", gubg::hr(node.all_tags));
		return os;
	}

} 