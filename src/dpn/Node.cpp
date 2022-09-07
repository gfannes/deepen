#include <dpn/Node.hpp>

#include <gubg/OnlyOnce.hpp>

namespace dpn { 

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

	bool Node::has_matching_tags(const Tags &wanted_tags) const
	{
		for (const auto &[key,value]: wanted_tags)
		{
			const auto it = total_tags.find(key);
			if (it == total_tags.end())
				return false;
			if (!it->second.count(value))
				return false;
		}
		return true;
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

} 