#ifndef HEADER_dpn_Filter_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Filter_hpp_ALREADY_INCLUDED

#include <dpn/types.hpp>
#include <dpn/meta/State.hpp>
#include <dpn/meta/Moscow.hpp>

#include <optional>

namespace dpn { 

	class Node;

	class Filter
	{
	public:
		TagSets any_tags;
		TagSets all_tags;
		TagSets none_tags;
		std::optional<meta::Status> status;
		std::optional<meta::Moscow> moscow;

		bool operator()(const Node &) const;

		Filter merge(const Filter &) const;
	};


} 

#endif