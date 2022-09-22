#ifndef HEADER_dpn_Node_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Node_hpp_ALREADY_INCLUDED

#include <dpn/Attribute.hpp>
#include <dpn/meta/State.hpp>
#include <dpn/meta/Moscow.hpp>
#include <dpn/meta/Sequence.hpp>
#include <dpn/meta/Effort.hpp>
#include <dpn/meta/Duedate.hpp>
#include <dpn/meta/Urgency.hpp>
#include <dpn/meta/Command.hpp>
#include <dpn/meta/Tag.hpp>
#include <dpn/log.hpp>

#include <gubg/std/filesystem.hpp>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <variant>
#include <ostream>

namespace dpn { 

	class Node;

	using Path = std::vector<const Node *>;

	using Tags = std::map<std::string, std::string>;
	using TagSets = std::map<std::string, std::set<std::string>>;
	
	class Node
	{
	public:
		Node() {}
		Node(const std::string &text): text(text) {}

		std::string text;
		unsigned int depth = 0;
		bool is_heading = false;
		Attributes attributes;

		using Meta = std::variant<meta::State, meta::Moscow, meta::Sequence, meta::Effort, meta::Duedate, meta::Urgency, meta::Command, meta::Tag>;
		std::vector<Meta> metas;

		std::vector<Node> childs;

		std::optional<meta::State> my_state;
		std::optional<meta::State> agg_state;

		std::optional<meta::Moscow> my_moscow;
		meta::Moscow agg_moscow;

		std::optional<meta::Sequence> my_sequence;
		meta::Sequence agg_sequence;

		meta::Effort my_effort;

		meta::Effort filtered_effort;
		meta::Effort total_effort;
		meta::Effort tmp_effort_;

		std::optional<meta::Urgency> my_urgency;
		meta::Urgency agg_urgency;

		std::set<std::filesystem::path> my_includes;
		std::set<std::filesystem::path> my_requires;
		std::set<std::filesystem::path> my_dependencies() const;
		
		std::set<std::filesystem::path> all_dependencies;

		template <typename Ftor>
		void each_dependency(Ftor &&ftor, bool all) const
		{
			if (all)
			{
				for (const auto &fp: all_dependencies)
					ftor(fp);
			}
			else
			{
				for (const auto &set: {my_includes, my_requires})
				{
					for (const auto &fp: set)
						ftor(fp);
				}
			}
		}

		Tags my_tags;
		TagSets all_tags;

		std::string path(const Path &, char sep = '/') const;

		bool has_matching_tags(const TagSets &wanted_tags, bool on_empty) const;

		template <typename Meta>
		const Meta *get() const
		{
			const Meta *ptr = nullptr;
			for (const auto &meta: metas)
			{
				if (auto *p = std::get_if<Meta>(&meta))
				{
					if (ptr)
						log::warning() << "Found multiple occurences for metadata " << meta::Traits<Meta>::type_name() << ", taking last item." << std::endl;
					ptr = p;
				}
			}
			return ptr;
		}
	};

	using Nodes = std::vector<Node>;

	std::string to_string(const Path &, char sep = '/');

	std::ostream &operator<<(std::ostream &, const Node &);

} 

#endif