#include <dpn/Node.hpp>

#include <gubg/OnlyOnce.hpp>

namespace dpn { 

	std::set<std::filesystem::path> Node::my_dependencies() const
	{
		auto res = my_includes;
		{
			auto copy = my_requires;
			res.merge(copy);
		}
		return res;
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

	bool Node::has_matching_tags(const TagSets &wanted_tags, bool on_empty) const
	{
		if (wanted_tags.empty())
			return on_empty;
		for (const auto &[key,wanted_values]: wanted_tags)
		{
			const auto it = all_tags.find(key);
			if (it == all_tags.end())
				return false;

			bool found_match = false;
			for (const auto &value: wanted_values)
			{
				if (it->second.count(value))
					// If one of the values for a given key matches, we have a match
					found_match = true;
			}
			if (!found_match)
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

	std::ostream &operator<<(std::ostream &os, const Node &node)
	{
		os << "[Node](text:" << node.text << "){\n";
		if (!node.all_tags.empty())
		{
			os << "  [all_tags]{\n";
			for (const auto &[key,values]: node.all_tags)
			{
				if (!values.empty())
				{
					os << "    [" << key << "]";
					for (const auto &value: values)
						os << "(" << value << ")";
					os << "\n";
				}
			}
			os << "  }\n";
		}
		os << "}\n";
		return os;
	}

} 