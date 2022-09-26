#include <dpn/Filter.hpp>
#include <dpn/Node.hpp>

#include <gubg/mss.hpp>

namespace dpn { 

	bool matches_any(const TagSets &tags, const TagSets &wanted_tags, bool on_empty)
	{
		if (wanted_tags.empty())
			return on_empty;

		for (const auto &[key,wanted_values]: wanted_tags)
		{
			const auto it = tags.find(key);
			if (it == tags.end())
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
	bool matches_all(const TagSets &tags, const TagSets &wanted_tags, bool on_key_not_found)
	{
		for (const auto &[key,wanted_values]: wanted_tags)
		{
			const auto it = tags.find(key);
			if (it == tags.end())
				return on_key_not_found;

			bool found_match = true;
			for (const auto &value: wanted_values)
			{
				if (!it->second.count(value))
					// If one of the values for a given key is not found, we have no match
					found_match = false;
			}
			if (!found_match)
				return false;
		}

		return true;
	}

	bool Filter::operator()(const Node &node) const
	{
		MSS_BEGIN(bool);

		MSS_Q(!node.text.empty() || !node.childs.empty() || !node.all_dependencies.empty());

		MSS_Q(matches_all(any_tags, all_tags, true));
		MSS_Q(matches_any(node.all_tags, any_tags, true));
		MSS_Q(matches_all(node.all_tags, all_tags, false));
		MSS_Q(!matches_any(node.all_tags, none_tags, false));

		if (status)
		{
			MSS_Q(!!node.agg_state);
			MSS_Q(node.agg_state->status == *status);
		}

		if (moscow)
		{
			MSS_Q(moscow->intersect(node.agg_moscow).any());
		}

		MSS_END();
	}

	Filter Filter::merge(const Filter &rhs) const
	{
		auto ret = *this;
		for (const auto &[key,values]: rhs.any_tags)
		{
			for (const auto &value: values)
				ret.any_tags[key].insert(value);
		}
		for (const auto &[key,values]: rhs.all_tags)
		{
			for (const auto &value: values)
				ret.all_tags[key].insert(value);
		}
		return ret;
	}

} 