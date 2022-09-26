#include <dpn/Filter.hpp>
#include <dpn/Node.hpp>

#include <gubg/mss.hpp>

namespace dpn { 

	bool Filter::operator()(const Node &node) const
	{
		MSS_BEGIN(bool);

		MSS_Q(!node.text.empty() || !node.childs.empty() || !node.all_dependencies.empty());

		MSS_Q(node.has_matching_tags(incl_tags, true));
		MSS_Q(!node.has_matching_tags(excl_tags, false));

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
		for (const auto &[key,values]: rhs.incl_tags)
		{
			for (const auto &value: values)
				ret.incl_tags[key].insert(value);
		}
		return ret;
	}

} 